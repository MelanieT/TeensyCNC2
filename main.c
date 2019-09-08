// TeensyCNC
// Copyright 2016 Matt Williams
//
// g-code interpreter code is mostly based on RepRap and Marginally Clever's "How to make a Arduino CNC", though I've made a lot of changes.
// USB CDC virtual serial is from Freescale's Processor Expert (I'm not crazy enough to write that from scratch!)
// Everything else is taken from some of my other projects or from scratch.
//
// This all can be improved drastically, but it's a well working CNC example. G-code interpreter needs syntax checking, badly.

// Sep 9, 2018 - Modifications made by Alun Jones (macros, pen up/down on Z, bootloader entry, job tracing, help, etc!)
// Sep 5, 2019 - Extensive rewrite by Wayne Holder (reversed Y axis and added new button handler and g-code parser)

#include "MK20D10.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include "pwm.h"
#include "motor.h"
#include "usb_dev.h"
#include "usb_serial.h"

// External system tick counter (driven by ARM systick interrupt and counts in microseconds)
extern volatile uint32_t Tick;

// Axis' target steps (absolute) and encoder counts
extern volatile int32_t Target[2];
extern volatile int32_t EncoderPos[2];

float max(float a, float b) {
  return a > b ? a : b;
}

// Simple USB CDC text printing
#define INFO(x)    cdc_print("* " x "\r\n")
#define RESULT(x)  cdc_print(x "\r\n")

// Cricut Mini steps per inch, this is triggering on both quadrature encoder edges, so it's actually 4X
#define X_STEPS_PER_INCH (2868.6132f)
#define Y_STEPS_PER_INCH (6876.0096f)

// Max steps to the cut area limit on a 8.5 inch wide cutting mat
#define X_MAT_STEPS 24150           // 2,841.176 steps/inch

// Arc section line length, the smaller the number, the finer the facet on arcs
#define CURVE_SECTION_INCHES 0.005f

// Unit scaler, 1.0 is no scale (inches), 1/25.4 is metric working units
float scale_to_inches = 1.0f;       // Default to inches at start.

// Current, Target, and Delta axis units (working units, inches, MM, etc)
float   posX = 0;                   // Current X Position
float   posY = 0;                   // Current Y Position
float   posZ = 0;                   // Current Z Position
float   targetX = 0;                // Target X Position
float   targetY = 0;                // Target Y Position
float   deltaX = 0;                 // Delta X
float   deltaY = 0;                 // Delta Y
float   distance = 0;               // Delta Distance

// Delta axis steps (machine units, steps)
int32_t dxSteps = 0;                // Delta X steps
int32_t dySteps = 0;                // Delta Y steps

// Axis direction flags {Y, X}
int8_t  dirX = 1;                   // X Axis step direction
int8_t  dirY = -1;                  // Y Axis step direction

float   feedrate = 90.0f;           // Feedrate, default is 90 in/min
bool    headDown = false;           // Head Solenoid state, true is down, else up
bool    abs_mode = true;            // Motion mode, false = incremental; true = absolute
bool    matLoaded = false;          // True if Cutting Mat is at loaded position, else false

#define MAX_COMMAND 128             // Max size of Incoming serial buffer

enum {
    NO_CANCEL = 0,
    BUTTON_CANCEL = 1,
    SOFTSTOP_CANCEL = 2
} cancelling = NO_CANCEL;

// Function prototypes for local functions
void    LoadYAxis (void);
void    EndJob (void);
uint8_t getButton ();

// Some helpful printing functions.
void cdc_print (char *s) {
  usb_serial_write(s, strlen(s));
  usb_serial_flush_output();
}

// Simple delay using ARM systick, set up for microseconds (see startup.c)
void DelayUS (uint32_t us) {
  uint32_t _time = Tick;
  while ((Tick - _time) < us);
}

// Same delay, only scaled 1000x for milliseconds
void DelayMS (uint32_t ms) {
  uint32_t _time = Tick;
  while ((Tick - _time) < (ms * 1000)) {
    getButton();
  }
}

// Resets CNC state to default
void SetJobDefaults (void) {
  feedrate = 90.0f;
  abs_mode = true;
  scale_to_inches = 1.0f;
}

// Resets Teensy into boot loader mode (for uploading new code)
void EnterBootLoader (void) {
  MotorDisable();
  INFO("Entering bootloader");
  DelayMS(500);
  __asm__ volatile("bkpt");
  // NOT REACHED!
  while (1);
}

void HeadUp (void) {
  if (headDown) {
    GPIOC->PCOR |= 0x0020U;                     // D13 - Solenoid Off (head up)
    DelayMS(300);                          // Delay, limits pen/knife slap/skipping
    headDown = false;
  }
}

void HeadDown (void) {
  if (!headDown) {
    GPIOC->PSOR |= 0x0020U;                     // D13 - Solenoid On (head down)
    DelayMS(300);                          // Delay, limits pen/knife slap/skipping
    headDown = true;
  }
}

// Calculates axis deltas, used any time target units change
void calculate_deltas (void) {
  deltaX = fabsf(targetX - posX);
  deltaY = fabsf(targetY - posY);
  distance = sqrtf(deltaX * deltaX + deltaY * deltaY);
  int32_t csX = (posX * X_STEPS_PER_INCH);
  int32_t csY = (posY * Y_STEPS_PER_INCH);
  int32_t tsX = (targetX * X_STEPS_PER_INCH);
  int32_t tsY = (targetY * Y_STEPS_PER_INCH);
  dxSteps = abs(tsX - csX);
  dySteps = abs(tsY - csY);
  dirX = (tsX - csX) > 0 ? 1 : -1;    // X axis
  dirY = (tsY - csY) > 0 ? 1 : -1;    // Y axis
}

// DDA line move code, optimized from "How to make a Arduino CNC", though it's pretty generic IMO

// Number of steps to make the move
int32_t dda_steps, dda_over;

// Which axis is dominant
bool  yOverX = false;

void set_target (float x, float y) {
  // Set the target location
  targetX = x;
  targetY = y;
  // Recalculate deltas
  calculate_deltas();
  // Set up total steps from dominant axis and set a flag for which one
  if (dxSteps > dySteps) {
    // X over Y
    yOverX = false;
    dda_steps = dxSteps;
    dda_over = dxSteps / 2;
  } else {
    // Y over X
    yOverX = true;
    dda_steps = dySteps;
    dda_over = dySteps / 2;
  }
}

// Sets the position (home)
void set_position (float x, float y) {
  posX = x;
  posY = y;
  calculate_deltas();
}

// Run the actual move, stripped bare, hopefully makes it faster
void dda_move (float feedRate) {
  // Calculate the length of the line to move and how long it will take to get there
  int32_t total = dxSteps > dySteps ? dxSteps : dySteps;
  uint32_t delay = ((distance * 60000000.0f) / feedRate) / total;
  // Loop until we're out of steps, or it's interrupted by button press
  while ((cancelling == NO_CANCEL) && dda_steps--) {
    if (yOverX) {
      Target[1] += dirY;
      dda_over += dxSteps;
    } else {
      Target[0] += dirX;
      dda_over += dySteps;
    }
    if (dda_over >= (yOverX ? dySteps : dxSteps)) {
      if (yOverX) {
        Target[0] += dirX;
        dda_over -= dySteps;
      } else {
        Target[1] += dirY;
        dda_over -= dxSteps;
      }
    }
    // Enforce soft limits since there's no safe hard stop.
    if ((Target[0] < 0) || (Target[0] > X_MAT_STEPS)) {
      cancelling = SOFTSTOP_CANCEL;
      break;
    }
    DelayUS(delay);
  }
  // Arrived at target
  posX = targetX;
  posY = targetY;
  // Recalculate deltas
  calculate_deltas();
}

/**
 *  G Codes Implemented by TeensyCNC:
 *    G00 - Rapid positioning
 *    G01 - Linear Interpolation
 *    G02 - Circular interpolation, clockwise
 *    G03 - Circular interpolation, counterclockwise
 *    G04 - Set Dwell (in seconds)
 *    G20 - Set Units to Inches
 *    G21 - Set Units to Millimeters
 *    G28 - Go home
 *    G30 - Go home in two steps
 *    G90 - Set Absolute Mode (default)
 *    G91 - Set Incremental Mode
 *    G92 - Set current position as home
 *
 *  M Codes Implemented by TeensyCNC:
 *    M2  - End of program
 *    M3  - Tool Down
 *    M4  - Tool Down
 *    M5  - Tool Up
 *    M7  - Tool Down
 *    M8  - Tool Up
 *    M30 - End of program
 *    M39 - Load Paper
 *    M40 - Eject Paper
 *
 *  Special M Codes
 *    M112 - Emergency stop / Enter bootloader
 *    M115 - Prints Build Info
 *
 *  Supported Parameters
 *    Fn.n - Feed Rate
 *    Pn.n - Pause in seconds (used by G4 command)
 *    Xn.n - X Coord
 *    Yn.n - Y Coord
 *    Zn.n - Z Coord
 *    In.n - I Coord (arc center X for arc segments)
 *    Jn.n - J Coord (arc center Y for arc segments)
 *
 *  Responses (terminated by "\r\n")
 *    ok        Normal response
 *    cancelled Job cancelled
 *    huh? G    Unknown "Gnn" code
 *    huh? M    Unknown "Mnn" code
 *    * --      Info Response
 */

void parseGcode (const char *line, int length) {
  float xVal = posX;
  float yVal = posY;
  float zVal = posZ;
  float iVal = 0;
  float jVal = 0;
  float pause = 1.0f;                         // Default pause time of 1 second
  int gVal = -1;
  char type = 0;
  // Process gcode line by line
  int state = 0;
  int ii = 0;
  while (ii < length) {
    switch (state) {
      case 0:                               // Waiting for Code
        switch (line[ii]) {
          case 'G':                         // Motion
          case 'F':                         // Feed Rate
          case 'M':                         // Misc. function
          case 'X':                         // Horizontal position (head left/right)
          case 'Y':                         // Vertical position (max in/out)
          case 'Z':                         // Tool Height
          case 'I':
          case 'J':
          case 'P':                         // Pause time (for G04 dwell command)
            type = line[ii];
            state = 1;                      // Parse parameter
            break;
          case '(':                         // Begin comment
            state = 2;
            break;
          case ';':                         // Skip rest of line as comment
            ii = length + 1;
            break;
        }
        ii++;
        break;
      case 1: {
        // Parse command parameter
        int intVal = 0;
        float decDivisor = 1;
        bool hasDecimal = false;
        bool isNegative = false;
        // Skip leading space, if any
        while (line[ii] == ' ' || line[ii] == '\t') {
          ii++;
        }
        // Check if number is prefixed by sign
        if (line[ii] == '-') {
          isNegative = true;
          ii++;
        } else if (line[ii] == '+') {
          ii++;
        }
        // Process digits and decimal point
        while (ii < length && ((line[ii] >= '0' && line[ii] <= '9') || line[ii] == '.')) {
          if (hasDecimal) {
            decDivisor *= 10;
          }
          if (line[ii] == '.') {
            hasDecimal = true;
            ii++;
          } else {
            intVal = (intVal * 10) + (line[ii++] - '0');
          }
        }
        if (isNegative) {
          intVal = -intVal;
        }
        switch (type) {
          case 'X':                                                     // X Axis Position
            if (abs_mode) {
              xVal = ((float) intVal / decDivisor) / scale_to_inches;
            } else {
              xVal += ((float) intVal / decDivisor) / scale_to_inches;
            }
            break;
          case 'Y':                                                     // Y axis Position
            if (abs_mode) {
              yVal = ((float) intVal / decDivisor) / scale_to_inches;
            } else {
              yVal += ((float) intVal / decDivisor) / scale_to_inches;
            }
            break;
          case 'Z':                                                     // Z Axis Position
            if (abs_mode) {
              zVal = ((float) intVal / decDivisor) / scale_to_inches;
            } else {
              zVal += ((float) intVal / decDivisor) / scale_to_inches;
            }
            if (zVal < 0) {
              HeadDown();
            } else {
              HeadUp();
            }
            break;
          case 'I':
            iVal = ((float) intVal / decDivisor) / scale_to_inches;
            break;
          case 'J':
            jVal = ((float) intVal / decDivisor) / scale_to_inches;
            break;
          case 'G':
            switch (intVal) {
              case 0:                                                   // Rapid positioning
              case 1:                                                   // Linear Interpolation
              case 2:                                                   // Clockwise arc segment
              case 3:                                                   // Counterclockwise arc segment
              case 4:                                                   // Set Dwell (in seconds)
                gVal = intVal;
                break;
              case 20:                                                  // Set Units to Inches
                scale_to_inches = 1.0f;
                break;
              case 21:                                                  // Set Units to Millimeters
                scale_to_inches = 25.4f;
                break;
              case 28:                                                  // Go home
                set_target(0.0f, 0.0f);
                dda_move(100.0f);
                DelayMS(100);
                // Zero out encoder and step positions
                EncoderPos[0] = 0;
                EncoderPos[1] = 0;
                Target[0] = 0;
                Target[1] = 0;
                break;
              case 90:                                                  // Set Absolute Mode (default)
                abs_mode = true;
                break;
              case 91:                                                  // Set Incremental Mode
                abs_mode = false;
                break;
              case 92:                                                  // Set current position as home
                set_position(0.0f, 0.0f);
                break;
              default:                                                  // Unknown command
                RESULT("huh? G");
                break;
            }
            break;
          case 'M':                                                     // Misc. function
            switch (intVal) {
              case 3:                                                   // Tool Down / spindle on clockwise
              case 4:                                                   // Tool Down / spindle on counterclockwise
              case 7:                                                   // Tool Down / mist coolant on
                HeadDown();
                break;
              case 5:                                                   // Tool Up / spindle stop
              case 8:                                                   // Tool Up / flood coolant on
                HeadUp();
                break;
              case 2:                                                   // End of program
              case 30:                                                  // End of program
                EndJob();
                break;
              case 39:                                                  // Load the cutting mat
                INFO("loading mat");
                matLoaded = false;
                LoadYAxis();
                break;
              case 40:                                                  // Eject the cutting mat
                INFO("unloading mat");
                matLoaded = true;
                LoadYAxis();
                break;
              case 112:
                EnterBootLoader();                                      // Emergency stop / Enter bootloader
                break;
              case 115:                                                 // Help
                INFO("-- TeensyCNC Rev 2 " __DATE__ " " __TIME__ " --"
                     "|Created by:| - Matt Williams|with additions by:| - Alun Jones (2018)| - Wayne Holder (2019)");
                break;
              default:                                                  // Unknown command
                RESULT("huh? M");
                break;
            }
            break;
          case 'F':                                                     // Feed Rate
            feedrate = ((float) intVal / decDivisor) / scale_to_inches;
            break;
          case 'P':
            pause = (float) intVal / decDivisor;
            break;
          case 'T':                                                     // Tool Select
            break;
        }
        state = 0;
      } break;
      case 2:                                                           // Process () comment
        while (ii < length && line[ii] != ')') {
          ii++;
        }
        if (line[ii] == ')') {
          ii++;
        }
        state = 0;
        break;
    }
  }
  // Get here when line processed
  switch (gVal) {
    case 0:                               // Move To new Location with Head Up at fast feed rate
      HeadUp();
      set_target(xVal, yVal);
      dda_move(250);
      break;
    case 1:                               // Line To new Location with Head Down at defined feed rate
      HeadDown();
      set_target(xVal, yVal);
      dda_move(feedrate);
      break;
    case 2:                               // Clockwise Arc Segment
    case 3: {                             // Counterclockwise Arc Segment
      // Get arc X/Y center (I/J)
      float cX = iVal + posX;
      float cY = jVal + posY;
      // Calculate the cross product
      float aX = posX - cX;
      float aY = posY - cY;
      float bX = xVal - cX;
      float bY = yVal - cY;
      float start_angle, end_angle;
      // CW or CCW
      if (gVal == 2) {
        // Find the starting and ending angle between the two points
        start_angle = atan2f(bY, bX);
        end_angle = atan2f(aY, aX);
      } else {
        start_angle = atan2f(aY, aX);
        end_angle = atan2f(bY, bX);
      }
      // Ensure we're still in range for radians
      if (end_angle <= start_angle) {
        end_angle += 2.0f * 3.1415926f;
      }
      // Calculate the total angle and radius of the arc
      float angle = end_angle - start_angle;
      float radius = sqrtf(aX * aX + aY * aY);
      // Calculate the number of steps, CURVE_SECTION_INCHES = lenth of line segment that forms the circle
      int steps = (int) ceilf(max(angle * 2.4f, (radius * angle) / CURVE_SECTION_INCHES));
      // Run it out
      for (int s = 1; s <= steps; s++) {
        float sf = (float) ((gVal == 3) ? s : steps - s) / (float) steps; // Step fraction, invert 's' for CCW arcs
        float theta = start_angle + angle * sf;
        set_target(cX + cosf(theta) * radius, cY + sinf(theta) * radius);
        dda_move(feedrate);
      }
    } break;
    case 4:                               // Dwell for P seconds
      DelayMS((int) (pause * 1000.0f));
      break;
  }
  switch (cancelling) {
    case NO_CANCEL:
      RESULT("ok");
      break;
    case SOFTSTOP_CANCEL:
      INFO("Head went out of bounds!");
      RESULT("cancelled");
      EndJob();
      break;
    case BUTTON_CANCEL:
      RESULT("cancelled");
      break;
    default:
      INFO("illegal 'cancelling' state");
      break;
  }
}

// Simple hard-stop axis homing function.  It's a bit of a hack, but works.
void HomeXAxis (void) {
  int32_t prevcount = 0, prevtime = 0;
  // Make sure the head is up.
  HeadUp();
  // Disable motor drive PID loop
  MotorDisable();
  // Store current X encoder position
  prevcount = EncoderPos[0];
  // Store current tick
  prevtime = Tick;
  // Drive the X motor home with enough torque to move it at a good pace, but not so much that it can't be stopped by the hard-stop.
  MotorCtrlX(-40000);
  // Let it move for a few ticks to generate some delta
  DelayMS(10);
  while (1) {
    // Velocity of motion over 1mS (1000uS)
    if ((Tick - prevtime) > 1000) {
      // Calculate the delta position from the last mS
      int32_t dC = abs(EncoderPos[0] - prevcount);
      // If the velocity drops below 1 step/mS, we've hit the hard-stop and drop out of the loop
      if (dC < 1) {
        break;
      }
      // Otherwise, update the previous position/time and continue on
      prevcount = EncoderPos[0];
      prevtime = Tick;
    }
  }
  // Stop the motor and let it settle
  MotorCtrlX(0);
  DelayMS(100);
  // Zero out encoder and step positions
  EncoderPos[0] = 0;
  EncoderPos[1] = 0;
  Target[0] = 0;
  Target[1] = 0;
  // Zero out the CNC position
  set_position(0.0f, 0.0f);
  // Let it settle again and reenable the PID loop
  DelayMS(100);
  MotorEnable();
  // We're home!
}

void LoadYAxis (void) {
  HomeXAxis();
  if (matLoaded) {
    // Run out enough to fully eject the cutting mat
    INFO("Unloading");
    set_position(0.0f, 0.0f);
    set_target(0.0f, -14.0f);
    dda_move(250.0f);
    set_position(0.0f, 0.0f);
    matLoaded = false;
    GPIOD->PCOR |= 0x0010U;                     // Teensy D6 - Grn Load LED D1 (Off)
  } else {
    // Load enough to put tool upper/left position
    INFO("Loading");
    set_position(0.0f, 0.0f);
    set_target(0.0f, 1.75f);
    dda_move(50.0f);
    set_position(0.0f, 0.0f);
    matLoaded = true;
    GPIOD->PSOR |= 0x0010U;                     // Teensy D6 - Grn Load LED D1 (On)
  }
  INFO("Done");
}

void EndJob (void) {
  HeadUp();
  // Return home at end of job, hopefully future will allow home position to be retained after load/unloads
  set_target(0.0f, 0.0f);
  dda_move(100.0f);
  DelayMS(100);
  // Zero out encoder and step positions
  EncoderPos[0] = 0;
  EncoderPos[1] = 0;
  Target[0] = 0;
  Target[1] = 0;
}

#define LOAD_SHORT_PRESS    1
#define LOAD_LONG_PRESS     2
#define POWER_SHORT_PRESS   3
#define POWER_LONG_PRESS    4

uint8_t getButton () {
  static uint32_t lastCheck = 0;
  static uint8_t  loadState = 0;
  static uint16_t loadLong = 0;
  static uint8_t  powerState = 0;
  static uint16_t powerLong = 0;
  if ((Tick - lastCheck) > 10 * 1000) {
    // Check buttons every 10 ms
    bool load = !(GPIOD->PDIR & 0x0002U);       // Teensy D14 - Load Button
    switch (loadState) {
      case 0:                                   // Waiting for Load Button press
        if (load) {
          loadState++;                          // Load Pressed
        }
        loadLong = 0;
        break;
      case 1:                                   // Wait for release, or long press
        if (!load) {
          loadState = 0;
          return LOAD_SHORT_PRESS;
        } else if (loadLong++ > 100) {          // If held for > 1 second, then long press
          loadState++;                          // Load Long Press
          return LOAD_LONG_PRESS;
        }
        break;
      case 2:                                   // Wait for release
        if (!load) {
          loadState = 0;
        }
        break;
    }
    bool power = !(GPIOD->PDIR & 0x0080U);      // Teensy D5 - Power Button
    switch (powerState) {
      case 0:                                   // Waiting for Power Button press
        if (power) {
          powerState++;                         // Power Pressed
        }
        powerLong = 0;
        break;
      case 1:                                   // Wait for release, or long press
        if (!power) {
          powerState = 0;
          return POWER_SHORT_PRESS;
        } else if (powerLong++ > 100) {         // If held for > 1 second, then long press
          powerState++;                         // Power Long Press
          return POWER_LONG_PRESS;
        }
        break;
      case 2:                                   // Wait for release
        if (!power) {
          powerState = 0;
        }
        break;
    }
    lastCheck = Tick;
  }
  return 0;
}

int main (void) {
  uint32_t lastactive = 0;
  char    lineBuf[MAX_COMMAND];
  uint8_t charCount = 0;
  // Head up/down Solenoid uses Teensy D13 (PTC5 output, also Teensy's onboard LED)
  PORTC->PCR[5] = PORT_PCR_MUX(1);         // Set PORTC_PCR5 MUX Field to GPIO
  GPIOC->PDDR |= 0x0020U;                     // Teensy D13 - Solenoid (set as output)
  GPIOC->PCOR |= 0x0020U;                     // Teensy D13 - Solenoid Off

  // Enable control of D1 Grn Power LED/\, Encoders and Motors using Teensy D8 (PTD3 output, PIC14 Pin 7)
  PORTD->PCR[3] = PORT_PCR_MUX(1);         // Set PORTD_PCR3 MUX Field to GPIOR
  GPIOD->PDDR |= 0x0008U;                     // Teensy D8 - Grn Power LED D3 (set as output)
  GPIOD->PSOR |= 0x0008U;                     // Teensy D8 - Grn Power LED D3 (Must be On to enable Encoders & Motors)

  // Enable control of D2 Red Power LED using Teensy D7 (PTD2 output, PIC14 Pin 6)
  PORTD->PCR[2] = PORT_PCR_MUX(1);         // Set PORTD_PCR2 MUX Field to GPIO
  GPIOD->PDDR |= 0x0004U;                     // Teensy D7 - Red Power LED D2 (set as output)
  GPIOD->PCOR |= 0x0004U;                     // Teensy D7 - Red Power LED D2 (Off)

  // Enable control of D3 Grn Load LED using Teensy D6 (PTD4 output, PIC14 Pin 5)
  PORTD->PCR[4] = PORT_PCR_MUX(1);         // Set PORTD_PCR4 MUX Field to GPIO
  GPIOD->PDDR |= 0x0010U;                     // Teensy D6 - Grn Load LED D1 (set as output)
  GPIOD->PCOR |= 0x0010U;                     // Teensy D6 - Grn Load LED D1 (Off)

  // Enable Power Button as Input using Teensy D5 (PTD7 input, PIC28 Pin 10)
  PORTD->PCR[7] = PORT_PCR_MUX(1);         // Set PORTD_PCR7 MUX Field to GPIO
  GPIOD->PDDR &= ~0x0080U;                    // Teensy D5 - Power Button (set as input)

  // Enable Load Button as Input using Teensy D14 (PTD1 input, PIC14 Pin 14)
  PORTD->PCR[1] = PORT_PCR_MUX(1);         // Set PORTD_PCR1 MUX Field to GPIO
  GPIOD->PDDR &= ~0x0002U;                    // Teensy D14 - Load Button (set as input)

  // Initialize X/Y motor PWM channels, set 0 duty (FFFFh = 0%, 0 = 100%)
  PWM_Init();
  PWM_SetRatio(0x00, 0xFFFF);
  PWM_SetRatio(0x01, 0xFFFF);
  PWM_SetRatio(0x05, 0xFFFF);
  PWM_SetRatio(0x06, 0xFFFF);
  Motor_Init();                               // Initialize motor PID control and encoder interrupts
  usb_init();                                 // Initialize USB CDC virtual serial device
  HomeXAxis();                                // Home the X axis
  SetJobDefaults();                           // Setup defaults
  // Read and process incoming gcode
  while (true) {
    bool idle = (Tick - lastactive) > 250000;
    if (idle && (cancelling != NO_CANCEL)) {
      cancelling = NO_CANCEL;
    }
    switch (getButton()) {
    case LOAD_SHORT_PRESS:
      INFO("Load Button was pressed");
      if (idle) {
        LoadYAxis();
      } else {
        INFO("Cancelling");
        cancelling = BUTTON_CANCEL;
        EndJob();
        RESULT("cancelled");
        lastactive = Tick;
      }
      break;
    case LOAD_LONG_PRESS:
      EnterBootLoader();
      break;
    case POWER_SHORT_PRESS:
      // Todo: implement motor/encoder power on/off
      INFO("POWER_SHORT_PRESS");
      break;
    case POWER_LONG_PRESS:
      // Todo: implement motor/encoder power on/off
      INFO("POWER_LONG_PRESS");
      break;
    }
    uint32_t incoming = usb_serial_available();
    while (incoming-- > 0) {
      char cc = (char) usb_serial_getchar();
      if (cancelling == NO_CANCEL) {
        if (cc == '\n') {
          if (charCount > 0) {
            parseGcode(lineBuf, charCount);
          }
          charCount = 0;
        } else if (cc >= ' ' && charCount < sizeof(lineBuf) - 1) {
          lineBuf[charCount++] = (char) toupper(cc);
          lineBuf[charCount] = 0;
        }
      }
      lastactive = Tick;
    }
  }
}
