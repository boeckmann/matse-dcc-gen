//
// Created by bernd on 28.05.18.
//


#include <avr/interrupt.h>
#include "bitstream.h"
#include "track.h"
#include "train.h"

#define NUM_SYNC_BITS 17
#define SHORT_PULSE 115
#define LONG_PULSE 199


typedef enum {
    FIRST_HALF,
    SECOND_HALF
} PeriodState;

typedef enum {
    SYNC,
    DATA_START,
    DATA
} PacketState;

typedef enum {
    BIT_ZERO,
    BIT_ONE
} Bit;


ISR (TIMER0_COMPA_vect)
{
    static uint8_t data_bit_pos = 0;
    static uint8_t sync_bit_pos = 0;

    static PacketState packet_state = SYNC;
    static Bit current_bit = BIT_ZERO;
    static PeriodState period_state = FIRST_HALF;

    static BitStream *stream;
    static uint8_t *data;
    static uint8_t data_bit_length;

    track_a_toggle_polarity();

    if (current_bit) OCR0A = SHORT_PULSE;   // schedule next timer
    else OCR0A = LONG_PULSE;                // 58us for logic one
                                            // >=100us for logic zero

    if (period_state == SECOND_HALF) {
        period_state = FIRST_HALF;

        switch (packet_state) {
            case SYNC:
                current_bit = BIT_ONE;
                if (sync_bit_pos == NUM_SYNC_BITS - 1) {
                    packet_state = DATA_START;
                    sync_bit_pos = 0;

                    //PORTB ^= 1 << PORTB1; // Toggle input 
                    stream = trains_schedule_next_bitstream();
                    data = stream->data;
                    data_bit_length = stream->length << 3;
                    data_bit_pos = 0;
                    //PORTB ^= 1 << PORTB1; // Toggle input 
                } else {
                    sync_bit_pos++;
                }
                break;

            case DATA_START:
                current_bit = BIT_ZERO;
                packet_state = DATA;
                break;

            case DATA:
                // Bit an aktueller Datenstromposition maskieren und testen
                if (data[data_bit_pos >> 3] & (1 << (7 - (data_bit_pos & 7)))) {
                    current_bit = BIT_ONE;
                } else {
                    current_bit = BIT_ZERO;
                }

                // An Bytegrenze testen ob Paketende erreicht ist, wenn ja
                // weiter mit Synchronisation, ansonsten Startbit (0) senden
                if ((data_bit_pos & 7) == 7) {
                    if (data_bit_pos == (data_bit_length - 1)) {
                        packet_state = SYNC;
                    } else {
                        packet_state = DATA_START;
                    }
                }

                data_bit_pos++;

                break;
        }
    }
    else {
        period_state = SECOND_HALF;
    }
}
