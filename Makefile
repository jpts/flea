#
#  Flea is a magic and cif plotting package written for Mississippi State
#  University by Edward Allen Luke
#

PROG = flea

CFILE = main.c buildbox.c output.c readtech.c readmagic.c readcif.c cifobjs.c\
		tilde.c devices.c PLdevice.c PSdevice.c VPdevice.c

OTHER = Makefile flea.h
MISC  = device.h
HFILE = flea.h device.h
ALL   = $(CFILE) $(OTHER) $(MISC)

OBJS =  main.o buildbox.o output.o readtech.o readmagic.o readcif.o cifobjs.o\
		tilde.o devices.o PLdevice.o PSdevice.o #VPdevice.o

# Use this CLIBS if you want to compile the Versatec device
# Also #define VERSATEC in the flea.h file
#CLIBS = clrlib.a -lF77 -lI77 -lm -lc #/msu/lib/plotters.a
CLIBS = -lm #/msu/lib/plotters.a


CFLAGS = -g 

CC  = gcc

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

$(PROG): $(OBJS) $(OTHER)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(CLIBS)


clean:
	rm -fr $(OBJS) $(PROG) core *~

tags:$(CFILE) $(HFILE)
	etags $(CFILE) $(HFILE)

