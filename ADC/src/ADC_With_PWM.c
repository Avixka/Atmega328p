#define F_CPU 8000000UL // Define CPU Clock Frequency as 8 MHz
#include <avr/io.h>
#include <util/delay.h>
// Note: <avr/interrupt.h> is not needed as no interrupts are used in this synchronous code.

// Global variable to store the calculated voltage in Volts (uses float for precision).
float Vin; 

/* -------------------------------------------------------------------------- */
/* ADC Functions                              */
/* -------------------------------------------------------------------------- */

// Initializes the Analog-to-Digital Converter (ADC) peripheral.
void ADC_Init(){

    // ADMUX (ADC Multiplexer Register): 
    // Selects AVCC (5V) as the voltage reference (REFS0 = 1).
    // The input channel is left at default (ADC0).
    ADMUX = (1<<REFS0); 

    // ADCSRA (ADC Control and Status Register A):
    // ADEN: Enable the ADC.
    // ADPS1 and ADPS2: Set Prescaler to 64 (8MHz / 64 = 125kHz ADC clock).
    ADCSRA = (1<<ADEN)|(1<<ADPS1)|(1<<ADPS2);
}

// Performs a single, synchronous (blocking) ADC conversion on the specified channel.
uint16_t ADC_Read(uint8_t channel) {
    // 1. Select Channel: Clear old MUX bits (& 0xF8) and insert new channel number (0-7) ( | & 0x07).
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);

    // 2. Start Conversion: Set the ADSC (ADC Start Conversion) bit to 1.
    ADCSRA |= (1<<ADSC);

    // 3. Wait: Wait actively until ADSC bit is cleared by the hardware (conversion complete).
    while(ADCSRA & (1<<ADSC));

    // 4. Return the 10-bit result (ADCL and ADCH combined by the 'ADC' macro).
    return ADC; 
}

/* -------------------------------------------------------------------------- */
/* PWM Functions                              */
/* -------------------------------------------------------------------------- */

// Initializes the Timer0 peripheral for Fast PWM output on Pin PD6 (OC0A).
void PWM_Init(){
    // DDRD: Set Pin PD6 as an OUTPUT (Crucial for the PWM signal to leave the chip).
    DDRD |= (1<<PD6);

    // TCCR0A (Timer/Counter Control Register 0 A):
    // COM0A1: Set Compare Output Mode (Non-inverting: Set at Bottom, Clear on Match).
    TCCR0A |= (1<<COM0A1);

    // WGM00 & WGM01: Set Waveform Generation Mode to Fast PWM (Mode 3).
    TCCR0A |= (1<<WGM00) | (1<<WGM01);

    // TCCR0B (Timer/Counter Control Register 0 B):
    // CS00 & CS01: Set Clock Select (Prescaler) to 64. 
    // (8MHz / 64 / 256 steps = ~488 Hz PWM frequency).
    TCCR0B |= (1<<CS00) | (1<<CS01);
}

/* -------------------------------------------------------------------------- */
/* Main Logic                                 */
/* -------------------------------------------------------------------------- */

int main(void){
    // Initialize the ADC and PWM peripherals.
    ADC_Init();
    PWM_Init();

    // DDRB: Set Pins PB0 and PB1 as OUTPUTS for the status LEDs.
    DDRB |= (1<<PB0) | (1<<PB1);

    while(1){
        
        // 1. READ SENSOR: Get 10-bit raw value from Channel 0 (ADC0).
        uint16_t adc_val = ADC_Read(0);

        // 2. CONVERT: Map the 10-bit value (0-1023) to a voltage (0.0V-5.0V).
        // Note: Using 1024.0 for max precision in the conversion factor.
        Vin=(adc_val*5.0)/1024.0;

        // 3. LOGIC: Control Status LEDs (Binary Logic: Red/Green Indicator)
        if(Vin<3.0){
            // If Vin < 3.0V (Low Voltage): Turn PB0 ON (Red/Warning) and PB1 OFF.
            PORTB |= (1<<PB0);  // Set PB0 to 1 (Turns ON)
            PORTB &=~ (1<<PB1); // Clear PB1 to 0 (Turns OFF)
        }
        else if (Vin>3.0)
        {
            // If Vin > 3.0V (OK Voltage): Turn PB1 ON (Green/OK) and PB0 OFF.
            PORTB |= (1<<PB1);  // Set PB1 to 1 (Turns ON)
            PORTB &=~ (1<<PB0); // Clear PB0 to 0 (Turns OFF)
        }
        // Note: If Vin is exactly 3.0V, the LED state is maintained (no change).

        // 4. CONTROL: Map the ADC value (0-1023) to the PWM range (0-255).
        // (10-bit / 4 = 8-bit, which prevents overflow)
        uint16_t duty_cycle = adc_val/4;

        // 5. ACTUATE: Write the new duty cycle to the Output Compare Register.
        // The hardware takes over, and the PWM signal instantly updates on PD6.
        OCR0A = duty_cycle;
    }
}