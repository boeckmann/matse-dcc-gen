#ifndef TICKS_H
#define TICKS_H

#include <stdint.h>

// Primitiver 8-bit Zeitgeber, der im DCC Serviceinterrupt
// nach jedem gesendeten Paket um eins erhöht wird. Da des
// Senden eines DCC Paketes weniger als 10ms in Anspruch nimmt
// ist sichergestellt, dass dieser Wert mindestens alle 10ms
// erhöht wird. Die minimale Zeit bis zur nächsten Erhöhung beträgt
// ca. 5ms für ein drei-Byte Datenpaket
extern volatile uint8_t ticks;

#endif
