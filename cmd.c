#include "cmd.h"
#include "serial.h"
#include "train.h"
#include "util.h"
#include "version.h"
#include <ctype.h>
#include <stdint.h>
#include <string.h>

int cmd_emergency_stop( const char *cmd );
int cmd_train_process( uint16_t addr, const char *cmd );
int cmd_train_dcc_mode( uint16_t addr, const char *cmd );
int cmd_train_activate( uint16_t addr, const char *cmd );
int cmd_train_speed_and_dir( uint16_t addr, const char *cmd );
int cmd_train_function( uint16_t addr, const char *cmd );

int cmd_process( const char *cmd )
{
    uint16_t addr;

    if ( *cmd == 0 ) {
        return 0;
    }

    if ( *cmd == 'H' ) { // NOTHALT
        return cmd_emergency_stop( cmd + 1 );
    }
    else if ( isdigit( *cmd ) ) { // Kommando für Zugadresse
        addr = str_to_uint16( &cmd );
        if ( addr > 127 ) {
            return 0;
        }
        // Lange Adressen noch nicht unterstützt
        //if (addr > 10239) return 0;
        return cmd_train_process( addr, cmd );
    }
    else if ( *cmd == 'I' ) { // initialize and send version over serial
        serial_puts( version_string );
        serial_puts( "\r\n" );
        return 1;
    }

    return 0;
}

int cmd_emergency_stop( const char *cmd )
{
    if ( *cmd == '+' ) {
        trains_emergency_stop( 1 );
    }
    else if ( *cmd == '-' ) {
        trains_emergency_stop( 0 );
    }
    else {
        return 0;
    }

    return 1;
}

int cmd_train_process( uint16_t addr, const char *cmd )
{
    if ( *cmd == 0 ) {
        return 0;
    }

    switch ( *cmd ) {
    case 'A': // DCC Signal für Zug (de)aktivieren
        return cmd_train_activate( addr, cmd );

    case 'C': // DCC Modus setzen
        return cmd_train_dcc_mode( addr, cmd );

    case 'V': // Zug vorwärts-Geschw. setzen
    case 'R': // Zug rückwärts-Geschw. setzen
        return cmd_train_speed_and_dir( addr, cmd );

    case 'F': // Funktion ein-/ausschalten
        return cmd_train_function( addr, cmd );

    default:
        break;
    }

    return 0;
}

int cmd_train_dcc_mode( uint16_t addr, const char *cmd )
{
    cmd++;
    if ( *cmd == 0 ) {
        return 0;
    }

    if ( *cmd == '+' || *cmd == '0') { // 14 Fahrstufen
        Train *train = train_by_addr( addr );
        train_set_dcc_mode( train, DCC_MODE_14 );
    }
    else if ( *cmd == '-' || *cmd == '1' ) { // 28 Fahrstufen
        Train *train = train_by_addr( addr );
        train_set_dcc_mode( train, DCC_MODE_28 );
    }
    else if ( *cmd == '2' ) {
        Train *train = train_by_addr( addr );
        train_set_dcc_mode( train, DCC_MODE_128 );        
    }
    else {
        return 0;
    }

    return 1;
}

int cmd_train_activate( uint16_t addr, const char *cmd )
{
    cmd++;
    if ( *cmd == 0 ) {
        return 0;
    }

    if ( *cmd == '+' ) { // Zug aktivieren
        Train *train = train_by_addr( addr );
        train_activate( train );
    }
    else if ( *cmd == '-' ) { // Zug deaktivieren
        Train *train = train_by_addr( addr );
        train_deactivate( train );
    }
    else {
        return 0;
    }

    return 1;
}

int cmd_train_speed_and_dir( uint16_t addr, const char *cmd )
{
    char dir = *cmd++;
    if ( !isdigit( *cmd ) ) {
        return 0;
    }

    uint8_t speed = str_to_uint8( &cmd );

    Train *train = train_by_addr( addr );

    if ( train->dcc_mode == DCC_MODE_14 && speed > 14 ) {
        return 0;
    }
    else if ( train->dcc_mode == DCC_MODE_28 && speed > 28 ) {
        return 0;
    }
    else if ( speed > 126 ) {
        return 0;
    }

    if ( addr > 0 ) {
        if ( dir == 'V' ) {
            train_set_speed_and_dir( train, speed, TRAIN_FORWARD );
        }
        else {
            train_set_speed_and_dir( train, speed, TRAIN_BACKWARD );
        }
        return 1;
    }
    else {
        // set speed for all trains
        //trains_set_speed((uint8_t), speed);
    }
    return 0;
}

int cmd_train_function( uint16_t addr, const char *cmd )
{
    cmd++;
    if ( !isdigit( *cmd ) ) {
        return 0;
    }
    uint8_t f = str_to_uint8( &cmd );

    Train *train = train_by_addr( addr );

    if ( *cmd == '+' ) {
        train_enable_function( train, f );
    }
    else if ( *cmd == '-' ) {
        train_disable_function( train, f );
    }
    else {
        return 0;
    }

    return 1;
}
