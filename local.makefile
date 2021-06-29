
#LDFLAGS+= -u _printf_float

console:
	screen /dev/ttyUSB0 115200

flash: all
	#stm32flash -w build/${TARGET}.bin -g 0x0 /dev/ttyUSB0
	st-flash write build/${TARGET}.bin 0x8000000
boot:
	#stm32flash -g 0x0 /dev/ttyUSB0
	st-flash reset

reset: boot

size: all
	$(SZ) --format=berkeley build/*.o


VERSION:=$(shell grep -e "^\#define VERSION" Inc/my_config.h|\
	awk '{print $$3}')
SUBVERSION:=$(shell grep -e "^\#define SUBVERSION" Inc/my_config.h|\
	awk '{print $$3}')

binary:
	mkdir -p binary

firmware: all binary
	cp -a build/${TARGET}.bin binary/${TARGET}_v${VERSION}.${SUBVERSION}.bin
