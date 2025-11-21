//#define F_CPU 16000000UL // 16 MHz Clock
#include <avr/io.h>
#include <util/delay.h>

void ADC_Init() {
    // ADMUX: Select ADC0 (PC0) and Reference Voltage AVCC (5V)
    ADMUX = (1 << REFS0); 
    // ADCSRA: Enable ADC, Set Prescaler to 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read(uint8_t channel) {
    // Select channel (keep safety mask to not mess up REFS0)
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
    // Start Conversion
    ADCSRA |= (1 << ADSC);
    // Wait for conversion to finish (Blocking approach for simplicity)
    while (ADCSRA & (1 << ADSC));
    return ADC; // Return 10-bit result (0-1023)
}

void PWM_Init() {
    // Set PD6 (OC0A) as Output
    DDRD |= (1 << PD6);
    // Configure Timer0: Fast PWM mode, Non-inverting
    TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00); // Prescaler 64
}

int main(void) {
    ADC_Init();
    PWM_Init();

    while (1) {
        // 1. Read Sensor (Potentiometer)
        uint16_t adc_val = ADC_Read(0); // Read channel 0

        // 2. Logic: Map ADC (0-1023) to PWM (0-255)
        // Simple math: Divide by 4
        uint8_t duty_cycle = adc_val / 4;

        // 3. Actuate: Update PWM Register
        OCR0A = duty_cycle; 
        

        _delay_ms(10); // Small stability delay
    }
}