PWD:=$(shell pwd)

all:
	cd $(PWD)/armgcc; make

flash:
	cd $(PWD)/armgcc; make flash

flash_softdevice:
	cd $(PWD)/armgcc; make flash_softdevice

clean:
	cd $(PWD)/armgcc; make clean
