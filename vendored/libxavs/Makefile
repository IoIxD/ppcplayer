# Makefile

include config.mak

SRCS = common/mc.c common/predict.c common/pixel.c common/macroblock.c \
       common/frame.c common/dct.c common/cpu.c\
       common/common.c common/mdate.c common/csp.c common/set.c \
       common/quant.c \
       encoder/analyse.c encoder/me.c encoder/ratecontrol.c \
       encoder/set.c encoder/macroblock.c\
       encoder/cavlc.c encoder/encoder.c encoder/eval.c \
       encoder/lookahead.c

SRCCLI = xavs.c matroska.c muxers.c

# MMX/SSE optims
ifneq ($(AS),)
X86SRC = common/i386/cpu-a.asm \
         common/i386/dct-sse2.asm \
         common/i386/deblock.asm \
         common/i386/deblock_inter.asm \
         common/i386/deblock_intra.asm \
         common/i386/mc-a01.asm \
         common/i386/mc-a.asm \
         common/i386/pixel-a.asm \
         common/i386/pixel-sse2.asm \
         common/i386/predict-a.asm \
         common/i386/qpel.asm \
         common/i386/quant_sse2.asm 
SRCS += common/i386/dct-c.c \
        common/i386/mc-c.c \
        common/i386/predict-c.c

ifeq ($(ARCH),X86)
ARCH_X86 = yes
ASMSRC   = $(X86SRC) 
endif

ifeq ($(ARCH),X86_64)
ARCH_X86 = yes
ASMSRC   = $(X86SRC:-32.asm=-64.asm)
ASFLAGS += -DARCH_X86_64
endif

ifdef ARCH_X86
ASFLAGS += -Icommon/i386/
OBJASM  = $(ASMSRC:%.asm=%.o)
endif
endif

# AltiVec optims


ifneq ($(HAVE_GETOPT_LONG),1)
SRCS += extras/getopt.c
endif

OBJS = $(SRCS:%.c=%.o)
OBJCLI = $(SRCCLI:%.c=%.o)
DEP  = depend

.PHONY: all default fprofiled clean distclean install uninstall
default: $(DEP) xavs$(EXE)

libxavs.a: .depend $(OBJS) $(OBJASM)
	$(AR) rc libxavs.a $(OBJS) $(OBJASM)
	$(RANLIB) libxavs.a

$(SONAME): .depend $(OBJS) $(OBJASM)
	$(CC) -shared -o $@ $(OBJS) $(OBJASM) -Wl,-soname,$(SOFLAGS) $(LDFLAGS)

xavs$(EXE): $(OBJCLI) libxavs.a 
	$(CC) -o $@ $+ $(LDFLAGS)

xavsvfw.dll: libxavs.a $(wildcard vfw/*.c vfw/*.h)
	make -C vfw/build/cygwin

checkasm: tools/checkasm.o libxavs.a
	$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.asm common/i386/i386inc.asm 
	$(AS) $(ASFLAGS) -o $@ $<
#delete local/anonymous symbols, so they don't show up in oprofile
	-@ $(STRIP) -x $@

.depend: config.mak
	rm -f .depend
	$(foreach SRC, $(SRCS) $(SRCCLI), $(CC) $(CFLAGS) $(ALTIVECFLAGS) $(SRC) -MT $(SRC:%.c=%.o) -MM -g0 1>> .depend;)

config.mak:
	./configure

depend: .depend
ifneq ($(wildcard .depend),)
include .depend
endif

SRC2 = $(SRCS) $(SRCCLI)
# These should cover most of the important codepaths
OPT0 = --crf 30 -b1 -m1 -r1 --me dia 
OPT1 = --crf 18 -b2 -m3 -r3 --me hex -8 --cqm avs --direct spatial
OPT2 = --crf 24 -b3 -m6 -r6 --me umh -8 -w -t1 -A all --b-pyramid --b-rdo --mixed-refs

ifeq (,$(VIDS))
fprofiled:
	@echo 'usage: make fprofiled VIDS="infile1 infile2 ..."'
	@echo 'where infiles are anything that xavs understands,'
	@echo 'i.e. YUV with resolution in the filename,y4m,  or avisynth.'
else
fprofiled:
	$(MAKE) clean
	mv config.mak config.mak2
	sed -e 's/CFLAGS.*/& -fprofile-generate/; s/LDFLAGS.*/& -fprofile-generate/' config.mak2 > config.mak
	$(MAKE) xavs$(EXE)
	$(foreach V, $(VIDS), $(foreach I, 0 1 2, ./xavs$(EXE) $(OPT$I) --threads 1 $(V) -o $(DEVNULL) ;))
	rm -f $(SRC2:%.c=%.o)
	sed -e 's/CFLAGS.*/& -fprofile-use/; s/LDFLAGS.*/& -fprofile-use/' config.mak2 > config.mak
	$(MAKE)
	rm -f $(SRC2:%.c=%.gcda) $(SRC2:%.c=%.gcno)
	mv config.mak2 config.mak
endif

clean:
	rm -f $(OBJS) $(OBJASM) $(OBJCLI) $(SONAME) *.a xavs xavs.exe .depend TAGS
	rm -f checkasm checkasm.exe tools/checkasm.o tools/checkasm-a.o
	rm -f $(SRC2:%.c=%.gcda) $(SRC2:%.c=%.gcno)
	- sed -e 's/ *-fprofile-\(generate\|use\)//g' config.mak > config.mak2 && mv config.mak2 config.mak
	rm -f ./*~
	rm -f ./tags
	rm -f encoder/*~
	rm -f common/*~
	rm -rf test/

distclean: clean
	rm -f config.mak config.h xavs.pc

install: xavs$(EXE) $(SONAME)
	install -d $(DESTDIR)$(bindir) $(DESTDIR)$(includedir)
	install -d $(DESTDIR)$(libdir) $(DESTDIR)$(libdir)/pkgconfig
	install -m 644 xavs.h $(DESTDIR)$(includedir)
	install -m 644 libxavs.a $(DESTDIR)$(libdir)
	install -m 644 xavs.pc $(DESTDIR)$(libdir)/pkgconfig
	install xavs$(EXE) $(DESTDIR)$(bindir)
	$(RANLIB) $(DESTDIR)$(libdir)/libxavs.a
ifeq ($(SYS),MINGW)
	$(if $(SONAME), install -m 755 $(SONAME) $(DESTDIR)$(bindir))
else
	$(if $(SONAME), ln -sf $(SONAME) $(DESTDIR)$(libdir)/libxavs.$(SOSUFFIX))
	$(if $(SONAME), install -m 755 $(SONAME) $(DESTDIR)$(libdir))
endif
	$(if $(IMPLIBNAME), install -m 644 $(IMPLIBNAME) $(DESTDIR)$(libdir))

uninstall:
	rm -f $(DESTDIR)$(includedir)/xavs.h $(DESTDIR)$(libdir)/libxavs.a
	rm -f $(DESTDIR)$(bindir)/xavs $(DESTDIR)$(libdir)/pkgconfig/xavs.pc
	$(if $(SONAME), rm -f $(DESTDIR)$(libdir)/$(SONAME) $(DESTDIR)$(libdir)/libxavs.$(SOSUFFIX))

tags:
	ctags -R ./*

test:
	perl tools/regression-test.pl --version=head,current --options='$(OPT0)' --options='$(OPT1)' --options='$(OPT2)' $(VIDS:%=--input=%)

testclean:
	rm -f test/*.log test/*.avs
	$(foreach DIR, $(wildcard test/xavs-r*/), cd $(DIR) ; make clean ; cd ../.. ;)

