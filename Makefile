PWD:=$(shell pwd)

all:
	cd $(PWD)/armgcc; make

flash:
	cd $(PWD)/armgcc; make flash

clean:
	cd $(PWD)/armgcc; make clean
