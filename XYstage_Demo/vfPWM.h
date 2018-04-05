// reference: https://forum.arduino.cc/index.php?topic=346731.0

// Output 46kHz PWM on Pin 7 & Pin 5
// use timer TCC0 (10-bit resolution)
// default dutycycle is zero.

void PWM_setup()
{
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |          // Divide the 48MHz clock source by divisor 1: 48MHz/1=48MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4. 0~3 are used by arduino
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Enable the port multiplexer for the digital pin D7 & D5
  PORT->Group[g_APinDescription[7].ulPort].PINCFG[g_APinDescription[7].ulPin].bit.PMUXEN = 1;
  PORT->Group[g_APinDescription[5].ulPort].PINCFG[g_APinDescription[5].ulPin].bit.PMUXEN = 1;
  // Connect the TCC0 timer to digital output D7 & D5 - port pins are paired odd PMUO and even PMUXE
  // F & E specify the timers: TCC0, TCC1 and TCC2
  PORT->Group[g_APinDescription[6].ulPort].PMUX[g_APinDescription[6].ulPin >> 1].reg = PORT_PMUX_PMUXO_F;
  PORT->Group[g_APinDescription[2].ulPort].PMUX[g_APinDescription[2].ulPin >> 1].reg = PORT_PMUX_PMUXO_F;

  // Feed GCLK4 to TCC0 and TCC1
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;   // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Dual slope PWM operation: timers countinuously count up to PER register value then down 0
  REG_TCC0_WAVE |= TCC_WAVE_POL(0xF) |         // Reverse the output polarity on all TCC0 outputs
                    TCC_WAVE_WAVEGEN_DSBOTH;    // Setup dual slope PWM on TCC0
  while (TCC0->SYNCBUSY.bit.WAVE);               // Wait for synchronization

  // Each timer counts up to a maximum or TOP value set by the PER register,
  // this determines the frequency of the PWM operation: 
  REG_TCC0_PER = PWM_RESOLUTION;         // Set the frequency of the PWM on TCC0 to 48M/prescaler/2/this number
  while (TCC0->SYNCBUSY.bit.PER);                // Wait for synchronization
  
  // Set the PWM signal to output 50% duty cycle
  REG_TCC0_CC3 = 0;         // TCC0 CC3 (or CCB3) - on D7
  while (TCC0->SYNCBUSY.bit.CCB3);                // Wait for synchronization
  
  REG_TCC0_CC1 = 0;         //TCC0 CC1 (or CCB1) - on D5
  while (TCC0->SYNCBUSY.bit.CCB1);
  
  // Divide the 48MHz signal by 1 giving 48MHz (20.83ns) TCC0 timer tick and enable the outputs
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV1 |    // Divide GCLK4 by 1
                    TCC_CTRLA_ENABLE;             // Enable the TCC0 output
  while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
}
// has to be called after PWM_setup()
// 0~512
void setPWMDuty_M2(int dutyCycle)
{
  if (dutyCycle < 0) dutyCycle = 0;
  if (dutyCycle > PWM_RESOLUTION) dutyCycle = PWM_RESOLUTION;
  REG_TCC0_CCB3 = dutyCycle;         // TCC0 CC3 - on D7
}

void setPWMDuty_M1(int dutyCycle)
{
  if (dutyCycle < 0) dutyCycle = 0;
  if (dutyCycle > PWM_RESOLUTION) dutyCycle = PWM_RESOLUTION;
  REG_TCC0_CCB1 = dutyCycle;         // TCC0 CCB1 - on D5
}