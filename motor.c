// TeensyCNC
// Copyright 2016 Matt Williams
//
// Motor PID control and encoder handling

// Alun Jones - Interrupt disable/enable while enabling/disabling motor, was causing MCU crashing.

#include "MK20D10.h"
#include <stdlib.h>
#include "pwm.h"

#ifndef clamp
#define clamp(a, min, max) (((a)<(min))?(min):(((a)>(max))?(max):(a)))
#endif
const int8_t Quad_Table[4][4][4] =
    {
        {
            {0, 1, -1, 3},
            {-1, 0, 2,  1},
            {1, -2, 0, -1},
            {3,  -1, 1, 0}
        },
        {
            {0, 1, -1, -2},
            {-1, 0, 3,  1},
            {1, 3,  0, -1},
            {2,  -1, 1, 0}
        },
        {
            {0, 1, -1, 2},
            {-1, 0, 3,  1},
            {1, 3,  0, -1},
            {-2, -1, 1, 0}
        },
        {
            {0, 1, -1, 3},
            {-1, 0, -2, 1},
            {1, 2,  0, -1},
            {3,  -1, 1, 0}
        }
    };

/*
  0x0040 = 0000 0000 0100 0000 = PORT C6 - Encoder XA
  0x0080 = 0000 0000 1000 0000 = PORT C7 - Encoder XB

  0x0001 = 0000 0000 0000 0001 = PORT B0 - Encoder YB
  0x0002 = 0000 0000 0000 0010 = PORT B1 - Encoder YA
*/
// Slots encoder pin status into a bit field, as a look up into Quad_Table for quadrature directional information
// Returns 0, 1, 2, or 3, depending on which opto sensor is blocked and when.
#define XENCODER_GET_PINS() ((GPIOC->PDIR & 0x00C0U) >> 6U)    // Encoder XB/XA
#define YENCODER_GET_PINS() ((GPIOB->PDIR & 0x0003U) >> 0U)    // Encoder YA/YB

// Current encoder quadratic value
uint8_t encoderQuadX;
uint8_t encoderQuadY;

// Previous encoder quadratic value
uint8_t encoderPrevQuadX;
uint8_t encoderPrevQuadY;

// Encoder coords to target (these do the moving)
volatile int32_t targetX = 0;
volatile int32_t targetY = 0;

volatile int32_t encoderPosX;
volatile int32_t encoderPosY;

// Set X axis motor PWM, neg values run opposite direction
void MotorCtrlX (int32_t PWM) {
  if (PWM > 0) {
    PWM_SetRatio(0x05, clamp((uint16_t) 65535 - abs(PWM), 0, 65535));
    PWM_SetRatio(0x06, 65535);
  } else {
    PWM_SetRatio(0x05, 65535);
    PWM_SetRatio(0x06, clamp((uint16_t) 65535 - abs(PWM), 0, 65535));
  }
}

// Same, but Y axis
void MotorCtrlY (int32_t PWM) {
  if (PWM > 0) {
    PWM_SetRatio(0x00, clamp((uint16_t) 65535 - abs(PWM), 0, 65535));
    PWM_SetRatio(0x01, 65535);
  } else {
    PWM_SetRatio(0x00, 65535);
    PWM_SetRatio(0x01, clamp((uint16_t) 65535 - abs(PWM), 0, 65535));
  }
}

// X encoder interrupt
void __attribute__ ((interrupt)) Cpu_ivINT_PORTC (void) {
  // Check for interrupt flag for either input
  if ((PORTC->PCR[6] & PORT_PCR_ISF_MASK) || (PORTC->PCR[7] & PORT_PCR_ISF_MASK)) {
    // Clear the flag(s)
    PORTC->PCR[6] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[7] |= PORT_PCR_ISF_MASK;
    // Get the encoder status
    uint8_t c12 = XENCODER_GET_PINS();
    // Retreive directional data from quadrature lookup table
    int8_t new_step = Quad_Table[encoderPrevQuadX][encoderQuadX][c12];
    // Store the previous, last value
    encoderPrevQuadX = encoderQuadX;
    // Store the current, last value
    encoderQuadX = c12;
    if (new_step == 3) {            // 3 is an error
      // Ignore error
    } else if (new_step != 0) {     // It's good?
      encoderPosX += new_step;    // Count it in whatever direction it's going
    }
  }
}

// Y encoder interrupt, exactly as X axis
void __attribute__ ((interrupt)) Cpu_ivINT_PORTB (void) {
  if ((PORTB->PCR[0] & PORT_PCR_ISF_MASK) || (PORTB->PCR[1] & PORT_PCR_ISF_MASK)) {
    PORTB->PCR[0] |= PORT_PCR_ISF_MASK;
    PORTB->PCR[1] |= PORT_PCR_ISF_MASK;
    uint8_t c12 = YENCODER_GET_PINS();
    int8_t new_step = Quad_Table[encoderPrevQuadY][encoderQuadY][c12];
    encoderPrevQuadY = encoderQuadY;
    encoderQuadY = c12;
    if (new_step == 3) {
      // Ignore error
    } else if (new_step != 0) {
      encoderPosY += new_step;
    }
  }
}

// PID stuff

// Position multiplier
#define KP 5000.0f
// Derivative multiplier
#define KD 24000.0f

// Previous derivative error
int32_t lastErrorX = 0;
int32_t lastErrorY = 0;

void __attribute__ ((interrupt)) Cpu_ivINT_FTM1 (void) {
  // Is the overflow interrupt flag pending?
  if (FTM1->SC & FTM_SC_TOF_MASK) {
    // Clear flag
    FTM1->SC &= ~FTM_SC_TOF_MASK;

    // Run proportional control
    // find the error term of current position - target
    int32_t errorX = targetX - encoderPosX;
    int32_t errorY = targetY - encoderPosY;

    //generalized PID formula
    //correction = Kp * error + Kd * (error - prevError)
    MotorCtrlX(KP * errorX + KD * (float) (errorX - lastErrorX));
    MotorCtrlY(KP * errorY + KD * (float) (errorY - lastErrorY));

    // Store previous error
    lastErrorX = errorX;
    lastErrorY = errorY;
  }
}

// Sets PID interrupt to system clock, enabling it.
void MotorEnable (void) {
  lastErrorX = 0;
  lastErrorY = 0;
  __disable_irq();
  FTM1->SC = (FTM1->SC & (~(FTM_SC_CLKS_MASK & FTM_SC_TOF_MASK))) | (0x08U);
  FTM1->SC = FTM_SC_TOIE_MASK | FTM_SC_CLKS(0x02) | FTM_SC_PS(0x00);
  __enable_irq();
}

// Removes clock source from PID interrupt timer, disabling it.
// Also sets axis motors to 0 PWM.
void MotorDisable (void) {
  __disable_irq();
  FTM1->SC = (FTM1->SC & (~(FTM_SC_CLKS_MASK & FTM_SC_TOF_MASK))) | (0x00U);
  FTM1->SC = FTM_SC_TOIE_MASK | FTM_SC_CLKS(0x00) | FTM_SC_PS(0x00);
  __enable_irq();
  MotorCtrlX(0);
  MotorCtrlY(0);
}

void Motor_Init (void) {
  // Initialize enocder inputs with interrupts on both edges
  // PB0/PB1 = Y A/B encoder input
  PORTB->PCR[0] = (PORTB->PCR[0] & ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x06))) | PORT_PCR_MUX(0x01);
  PORTB->PCR[0] = (PORTB->PCR[0] & ~(PORT_PCR_IRQC(0x04))) | (PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x0B));
  PORTB->PCR[1] = (PORTB->PCR[1] & ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x06))) | PORT_PCR_MUX(0x01);
  PORTB->PCR[1] = (PORTB->PCR[1] & ~(PORT_PCR_IRQC(0x04))) | (PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x0B));
  NVIC_SetPriority(PORTB_IRQn, 0x50);
  NVIC_EnableIRQ(PORTB_IRQn);

  // PC6/PC7 = X A/B encoder input
  PORTC->PCR[6] = (PORTC->PCR[6] & ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x06))) | PORT_PCR_MUX(0x01);
  PORTC->PCR[6] = (PORTC->PCR[6] & ~(PORT_PCR_IRQC(0x04))) | (PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x0B));
  PORTC->PCR[7] = (PORTC->PCR[7] & ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x06))) | PORT_PCR_MUX(0x01);
  PORTC->PCR[7] = (PORTC->PCR[7] & ~(PORT_PCR_IRQC(0x04))) | (PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x0B));
  NVIC_SetPriority(PORTC_IRQn, 0x50);
  NVIC_EnableIRQ(PORTC_IRQn);

  // Initialize interrupt timer for PID control
  SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;

  // Set up mode register
  FTM1->MODE = FTM_MODE_FAULTM(0x00) | FTM_MODE_WPDIS_MASK;
  // Clear status and control register
  FTM1->SC = FTM_SC_CLKS(0x00) | FTM_SC_PS(0x00);
  // Clear counter initial register
  FTM1->CNTIN = FTM_CNTIN_INIT(0x00);
  // Reset counter register
  FTM1->CNT = FTM_CNT_COUNT(0x00);
  // Clear channel status and control register
  FTM1->CONTROLS[0].CnSC = 0x00;
  // Clear channel status and control register
  FTM1->CONTROLS[1].CnSC = 0x00;

  // Set up modulo register
  // Bus clock / Freq = FTM1_MOD
  // 36MHz / Freq = FTM1_MOD
  // MOD = 9 = 4000000Hz (4Mhz)
  FTM1->MOD = FTM_MOD_MOD(9 - 1);
  NVIC_SetPriority(FTM1_IRQn, 0x10);
  NVIC_EnableIRQ(FTM1_IRQn);

  // Set up status and control register
  FTM1->SC = FTM_SC_TOIE_MASK | FTM_SC_CLKS(0x02) | FTM_SC_PS(0x00);

  // Initialize encoder variables
  encoderQuadX = XENCODER_GET_PINS();
  encoderPrevQuadX = encoderQuadX;
  encoderQuadY = YENCODER_GET_PINS();
  encoderPrevQuadY = encoderQuadY;
}
