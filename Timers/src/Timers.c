#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h> // Essential for ISRs

// Global "Time" Variable (Must be volatile!)
volatile uint32_t System_Time_ms = 0;

void Timer1_Init() {
    // 1. Set Mode: CTC (Clear Timer on Compare Match)
    // WGM12 = 1 turns on CTC mode for Timer1
    TCCR1B |= (1 << WGM12);

    // 2. Set Prescaler: 64
    // CS11 = 1, CS10 = 1 sets prescaler to 64
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // 3. Set the Speed (The "Ceiling")
    // We calculated 249 for 1ms interval at 16MHz/64
    OCR1A = 124;

    // 4. Enable the Interrupt
    // OCIE1A: Output Compare Interrupt Enable 1A
    TIMSK1 |= (1 << OCIE1A);

    // 5. Enable Global Interrupts
    sei(); 
}

// --- THE INTERRUPT SERVICE ROUTINE ---
// This function runs automatically every 1 millisecond
ISR(TIMER1_COMPA_vect) {
    System_Time_ms++; // Just count up!
}

// Helper function to read time safely
uint32_t millis() {
    uint32_t time;
    // Disable interrupts briefly while reading to prevent data corruption
    cli(); 
    time = System_Time_ms;
    sei();
    return time;
}

int main(void) {
    // Setup LED on PB5 (Arduino Pin 13)
    DDRB |= (1 << PB5);
    
    Timer1_Init(); // Start the Heartbeat

    // Variables to track the last time tasks ran
    uint32_t Last_Blink_Time = 0;
    uint32_t Last_ADC_Time = 0;

    while (1) {
        // --- TASK 1: Blink LED every 1000ms ---
        // Non-blocking check!
        if (millis() - Last_Blink_Time >= 1000) {
            PORTB ^= (1 << PB5); // Toggle LED
            Last_Blink_Time = millis(); // Reset task timer
        }

        // --- TASK 2: Read Sensors every 100ms ---
        if (millis() - Last_ADC_Time >= 100) {
            // Imagine ADC_Read() is here...
            // This code runs 10 times faster than the blink task
            Last_ADC_Time = millis();
        }
    }
}