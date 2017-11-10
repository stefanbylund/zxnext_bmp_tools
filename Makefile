################################################################################
# Stefan Bylund 2017
#
# Makefile for compiling the Next BMP tools.
################################################################################

MKDIR := mkdir -p

RM := rm -rf

CP := cp

ZIP := zip -r -q

all:
	$(MKDIR) bin
	gcc -O2 -Wall -o bin/nextbmp src/nextbmp.c
	gcc -O2 -Wall -o bin/nextraw src/nextraw.c

distro:
	$(MAKE) clean all
	$(RM) tmp
	$(MKDIR) tmp/zxnext_bmp_tools
	$(CP) bin/* tmp/zxnext_bmp_tools
	$(CP) src/* tmp/zxnext_bmp_tools
	$(CP) README.md tmp/zxnext_bmp_tools
	$(RM) build/zxnext_bmp_tools.zip
	cd tmp; $(ZIP) ../build/zxnext_bmp_tools.zip zxnext_bmp_tools
	$(RM) tmp

clean:
	$(RM) bin tmp
