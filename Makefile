APP = longan-nano

OSDIR = mdepx

CMD = python3 -B ${OSDIR}/tools/emitter.py

all:
	@${CMD} -j mdepx.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/${APP}.elf obj/${APP}.hex
	@${CROSS_COMPILE}objcopy -O binary obj/${APP}.elf obj/${APP}.bin
	@${CROSS_COMPILE}size obj/${APP}.elf

debug:
	@${CMD} -d mdepx.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/${APP}.elf obj/${APP}.hex

clean:
	@rm -rf obj/*

include ${OSDIR}/mk/user.mk
