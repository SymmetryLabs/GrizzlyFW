CFLAGS+=-Wall -O2 -fomit-frame-pointer

all: libTLSF.a

LIBTLSF_DEP = TLSF_initialisation.o TLSF_malloc.o TLSF_free.o \
	TLSF_destructor.o TLSF_addblock.o TLSF_calloc.o TLSF_realloc.o \
	TLSF_debug_functions.o

dep:
	@if [ ! -d arch ] ; then \
		if [ `uname -m | sed -e "s/i.86/i386/" ` == "i386" ] ; then \
			ln -sf i386 arch ; \
		else \
			ln -sf generic-arch arch ; \
		fi \
	fi

libTLSF.a: dep $(LIBTLSF_DEP)
	@$(AR) -r libTLSF.a $(LIBTLSF_DEP)

TLSF_malloc.o: TLSF_malloc.c TLSF_def.h rtl_malloc.h TLSF_struct_functions.h

TLSF_free.o: TLSF_free.c TLSF_def.h rtl_malloc.h TLSF_struct_functions.h

TLSF_initialisation.o: TLSF_initialisation.c TLSF_def.h rtl_malloc.h

TLSF_addblock.o: TLSF_addblock.c TLSF_def.h rtl_malloc.h TLSF_struct_functions.h

TLSF_calloc.o: TLSF_calloc.c TLSF_def.h rtl_malloc.h

TLSF_destruction.o: TLSF_destruction.c TLSF_def.h rtl_malloc.h

TLSF_realloc.o: TLSF_realloc.c TLSF_def.h rtl_malloc.h

TLSF_debug_functions.o: TLSF_debug_functions.c TLSF_def.h rtl_malloc.h

regression_tests: dep regression_tests.o libTLSF.a
	$(CC) $(CFLAGS) -o regression_tests regression_tests.o -L./ -lTLSF

regression_tests.o: regression_tests.c TLSF_def.h rtl_malloc.h

clean:
	$(RM) -rf *.o *~ libTLSF.a regression_tests arch
