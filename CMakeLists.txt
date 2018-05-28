CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
SET(CMAKE_SYSTEM_NAME Generic)

SET(AVRBIN /usr/bin)			# <-- Toolchain
SET(CMAKE_C_COMPILER ${AVRBIN}/avr-gcc)
SET(CMAKE_CXX_COMPILER ${AVRBIN}/avr-g++)

#============================================================================================
PROJECT(berndcc-gen)		  		# <-- Prjektname

SET(FILES main.c serial.c serial.h train.c train.h bitstream.c bitstream.h cmd.c cmd.h util.c util.h isr_dcc.c version.c version.h)		  	# <-- Quelldatei(en)

SET(DEVICE atmega328p)			# <-- MCU
SET(FREQ 16000000)			# <-- MCU-Frequenz

SET(PROGRAMMER arduino)		# <-- Programmertype
SET(PORT usb)		         	# <-- USB bzw. Schnittstelle z.B. /dev/cu.usbserial....
SET(BAUD 57600)	        	# <-- Übertragungsrate

SET(EEPROM NO)	     	        	# <-- soll eine x.eep Datei erstellt werden?
SET(FLASH NO)		        	# <-- soll Geflasht werden?

#============================================================================================

SET(AVROBJCOPY avr-objcopy)		# <--     ""
SET(AVRSIZE avr-size)			# <--     ""
SET(AVRDUDE avrdude)			# <--     ""

SET(CMAKE_C_FLAGS  "-Os -mmcu=${DEVICE} -DF_CPU=${FREQ}UL -std=gnu99 -fshort-enums -Wl,--gc-sections")
SET(CMAKE_CXX_FLAGS "-Os -mmcu=${DEVICE} -DF_CPU=${FREQ}UL -fshort-enums  -Wl,--gc-sections")


#include_directories( /opt/local/avr/include)

SET(SOURCE_FILES ${FILES})

#============================================================================================

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${FILES})

ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME} ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex)

if(EEPROM)
    ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${AVRBIN}/${AVROBJCOPY} -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load"  --change-section-lma .eeprom=0 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.eep)
endif(EEPROM)

#ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${AVRBIN}/${AVRSIZE} --target=ihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.hex)

if(FLASH)
    ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${AVRBIN}/${AVRDUDE} -p${DEVICE} -c${PROGRAMMER} -P${PORT} -b${BAUD} -U flash:w:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.hex)
endif(FLASH)

