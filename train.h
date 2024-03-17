#ifndef TRAIN_H
#define TRAIN_H

#include <stdint.h>

enum { DCC_MODE_14, DCC_MODE_28, DCC_MODE_128 };

enum { TRAIN_BACKWARD, TRAIN_FORWARD };

enum {
    SPEED_AND_DIR,
    FUNCTION_0_4,
    FUNCTION_5_8,
    FUNCTION_9_12,
    FUNCTION_13_20,
    FUNCTION_21_28,
    FUNCTION_29_36,
    FUNCTION_37_44,
    FUNCTION_45_52,
    FUNCTION_53_60,
    FUNCTION_61_68
};

typedef struct Train {
    uint16_t addr;
    uint8_t speed;
    uint8_t direction;
    uint8_t functions[9]; // Functions 0-68
    uint8_t f_enabled;    // Welche Funktionspakete >F12 sind akiviert?
    uint8_t stream_type;  // Welches Paket wird gesendet?
    uint8_t active;       // Generierung von Zugpaketen aktiviert?
    uint8_t dcc_mode;

    struct Train *next;
} Train;

extern Train *trains;

Train *train_new( uint16_t addr );
Train *train_init( Train *train, uint16_t addr );

void train_activate( Train *train );
void train_deactivate( Train *train );

void train_set_dcc_mode( Train *train, uint8_t dcc_mode );
void train_set_speed_and_dir( Train *train, uint8_t speed, uint8_t dir );

void train_enable_function( Train *train, uint8_t f );
void train_disable_function( Train *train, uint8_t f );

Train *train_by_addr( uint16_t addr );


#endif // TRAIN_H
