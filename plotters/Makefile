CFLAGS = -g
OBJS = plotget.o

plotters.a:$(OBJS)
	ar r plotters.a $(OBJS)
	ranlib plotters.a

plotget.o:plotget.c
	cc $(CFLAGS) -c plotget.c
