all: test

TLSF_PATH=../src

CFLAGS+=-O2 -I$(TLSF_PATH) -L$(TLSF_PATH)

test: test.o $(TLSF_PATH)/rtl_malloc.h
	$(CC) $(CFLAGS) -o test test.o -lTLSF

test.o: test.c $(TLSF_PATH)/rtl_malloc.h

clean:
	$(RM) -rf *.o test *~
