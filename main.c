#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include <stddef.h>
#include <string.h>
#include "serial.h"

#define MAX_TRAINS  4       // maximum train number

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
    LOW,
    HIGH
} Bit;

typedef struct {
    uint8_t length;
    uint8_t data[8];
    volatile uint8_t active;
} BitStream;

#define TRAIN_FORWARD 1
#define TRAIN_BACKWARD 0

typedef struct {
    uint8_t addr;
    uint8_t speed;
    uint8_t direction;
    uint8_t f0;
    BitStream bitstream;
} Train;

static Train train01;

static Train trains[MAX_TRAINS];

volatile uint16_t hiqhfreq_counter = 0;

void wait_ms(uint16_t delay)
{
    if (delay < 1) delay = 1;
    uint16_t end = hiqhfreq_counter + delay;
    while (hiqhfreq_counter != end);
}

void update_checksum(BitStream *stream)
{
    uint8_t i = 0;
    uint8_t checksum = 0;

    for (; i < stream->length - 1; i++) {
        checksum ^= stream->data[i];
    }

    stream->data[stream->length-1] = checksum;
}


void train_update_bitstream(Train *train)
{
    BitStream *stream = &train->bitstream;

    while (stream->active);

    stream->length = 3;
    stream->data[0] = train->addr;

    uint8_t speed_and_dir = 0x40;
    if (train->direction) speed_and_dir |= 0x20;
    if (train->speed > 0) {
        speed_and_dir = speed_and_dir | (uint8_t)(train->speed + 2);
    }
    if (train->f0) speed_and_dir |= 0x10;
    stream->data[1] = speed_and_dir;

    update_checksum(stream);
}

void train_init(Train *train, uint8_t addr)
{
    train->addr = addr;
    train->f0 = 0;
    train->speed = 0;
    train->direction = 1;

    train->bitstream.active = 0;

    train_update_bitstream(train);
}

void train_set_f0(Train *train, uint8_t f0) {
    train->f0 = f0;
    train_update_bitstream(train);
}

void train_set_speed(Train *train, uint8_t speed) {
    if (speed <= 27) {
        train->speed = speed;
    } else {
        train->speed = 27;
    }

    train_update_bitstream(train);
}

void train_set_direction(Train *train, uint8_t dir)
{
    train->direction = dir;
}


void init_avr()
{
    TCCR0A = (1<<WGM01); // CTC Modus
    TCCR0B |= (1<<CS01); // Prescaler 8
    OCR0A = SHORT_PULSE;
    TIMSK0 |= (1<<OCIE0A); // Compare Interrupt für Timer 0 erlauben

    // 1000Hz counter initialisieren
    TCCR2A = (1<<WGM21);
    TCCR2B |= (1<<CS22);   // Prescaler 64
    OCR2A = 249;
    TIMSK2 |= (1<<OCIE2A);

    DDRB = 0xff;    // Port B als Ausgabe Konfigurieren
    PORTB = 1;      // Ausgabe initialisieren
}

static char cmd[16];

int handle_cmd(char *cmd)
{
    int len = strlen(cmd);
    int pos = 0;
    int addr;
    int speed;
    char dir;

    if (len - pos < 3) return 0;

    addr = (cmd[pos]-'0')*100 + (cmd[pos+1]-'0')*10 + (cmd[pos+2]-'0');

    if (addr > MAX_TRAINS) return 0;

    pos += 3;

    if (pos >= len) return 0;

    switch (cmd[pos]) {
        case 'V':
        case 'R':
            dir = cmd[pos++];
            if (len - pos < 2) return 0;
            speed = (cmd[pos]-'0')*10 + (cmd[pos+1]-'0');

            if (addr > 0) {
                if (dir == 'V') {
                    train_set_direction(&trains[addr-1], TRAIN_FORWARD);
                } else {
                    train_set_direction(&trains[addr-1], TRAIN_BACKWARD);
                }
                train_set_speed(&trains[addr-1], (uint8_t)speed);
            }
            else {
                // set speed for all trains
                //trains_set_speed((uint8_t), speed);
            }
            break;

        default:
            return 0;
    }

    return 1;
}

void main()
{
    init_avr();
    train_init(&train01, 1);
    train_set_f0(&train01, 1);
    train_set_speed(&train01, 6);
    //train_update_bitstream(&train01);

    serial_init(9600);

    sei();          // Global Interrupts aktivieren

    uint8_t speed = 6;

    serial_puts("READY\r\n");
    while(1) {
        //_delay_ms(5000);
        //if (speed == 6) speed = 10;
        //else speed = 6;
        //train_set_speed(&train01, speed);// Licht jede sekunde an/aus schalten
        //wait_ms(1);
        //PORTB ^= 1 << PORTB2;
        if (serial_recv_cmd(cmd, sizeof(cmd))) {
            if (handle_cmd(cmd)) {
                serial_puts("OK\r\n");
            } else {
                serial_puts("ERROR\r\n");
            }
            //serial_puts(cmd);
        }
    }
}

ISR (TIMER0_COMPA_vect)
{
    static uint8_t data_bit_pos = 0;
    static uint8_t sync_bit_pos = 0;

    static PacketState packet_state = SYNC;
    static Bit current_bit = LOW;
    static PeriodState period_state = FIRST_HALF;

    static BitStream *stream;
    static uint8_t *data;
    static uint8_t data_bit_length;


    PORTB ^= 1 << PORTB1 | 1 << PORTB0; // Toggle input

    if (current_bit) OCR0A = SHORT_PULSE;   // schedule next timer
    else OCR0A = LONG_PULSE;                // 58us for logic one
                                            // >=100us for logic zero

    if (period_state == SECOND_HALF) {
        period_state = FIRST_HALF;

        switch (packet_state) {
            case SYNC:
                current_bit = HIGH;
                if (sync_bit_pos == NUM_SYNC_BITS - 1) {
                    packet_state = DATA_START;
                    sync_bit_pos = 0;

                    stream = &train01.bitstream;
                    stream->active = 1;
                    data = stream->data;
                    data_bit_length = stream->length << 3;
                    data_bit_pos = 0;

                } else {
                    sync_bit_pos++;
                }

                break;

            case DATA_START:
                current_bit = LOW;
                packet_state = DATA;
                break;

            case DATA:
                if (data[data_bit_pos >> 3] & (1 << (7 - (data_bit_pos & 0x07)))) {
                    current_bit = HIGH;
                } else {
                    current_bit = LOW;
                }

                if ((data_bit_pos & 7) == 7) {
                    if (data_bit_pos == (data_bit_length - 1)) {
                        stream->active = 0;
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

ISR (TIMER2_COMPA_vect)
{
    sei();  // Interrupts reaktivieren damit Signal-ISR
            // nicht aus dem Tritt kommt
    hiqhfreq_counter++;
}
