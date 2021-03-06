#*=====================================================================*/
#*    serrano/prgm/project/bigloo/bde/bmem/Makefile                    */
#*    -------------------------------------------------------------    */
#*    Author      :  Manuel Serrano                                    */
#*    Creation    :  Sun Apr 20 09:34:24 2003                          */
#*    Last change :  Mon Oct 23 19:43:06 2017 (serrano)                */
#*    Copyright   :  2003-19 Manuel Serrano                            */
#*    -------------------------------------------------------------    */
#*    The makefile to build bmem                                       */
#*=====================================================================*/

#*---------------------------------------------------------------------*/
#*    The default configuration                                        */
#*---------------------------------------------------------------------*/
include ../../Makefile.buildconfig
include ../../Makefile.config
include ../../Makefile.misc

#*---------------------------------------------------------------------*/
#*    Objects                                                          */
#*---------------------------------------------------------------------*/
SRC		= bmem.scm function.scm tools.scm gc.scm type.scm \
                  html.scm param.scm thread.scm
INCLUDE		= html.sch html-private.sch
OBJECT		= $(SRC:%.scm=bmem/%.o)
LIBSRC		= init.c alloc.c gchook.c list.c symbol.c trace.c thread.c
LIBINCLUDE	= bmem.h esymbol.h
LIBOBJECT	= $(LIBSRC:%.c=lib/%.o)

POPULATION	= $(SRC:%=bmem/%) $(INCLUDE:%=bmem/%) \
                  $(LIBSRC:%=lib/%) $(LIBINCLUDE:%=lib/%) \
                  Makefile etc/bmem.css etc/bmemrun.in

#*---------------------------------------------------------------------*/
#*    Compilers & Flags                                                */
#*---------------------------------------------------------------------*/
IFLAGS		= -Ilib -I$(BOOTLIBDIR)
CMEMFLAGS	= -DBMEMDEBUG $(CPICFLAGS)
BMEMFLAGS	= $(BFLAGS) -g -I bmem $(EFLAGS)

#*---------------------------------------------------------------------*/
#*    Suffixes                                                         */
#*---------------------------------------------------------------------*/
.SUFFIXES:
.SUFFIXES: .scm .bgl .class .o .c .obj

#*---------------------------------------------------------------------*/
#*    Targets                                                          */
#*---------------------------------------------------------------------*/
.PHONY: bmem lib bmemrun bmem.css css

all: lib bmem bmemrun css

#*--- bmemrun ---------------------------------------------------------*/
bmemrun: $(BOOTBINDIR)/$(BMEMRUN_EXE)
$(BOOTBINDIR)/$(BMEMRUN_EXE): etc/bmemrun.in
	cat etc/bmemrun.in | \
            sed -e "s|<fildir>|$(LIBDIR)/$(FILDIR)|g" > \
            $(BOOTBINDIR)/$(BMEMRUN_EXE)
	chmod a+x $(BOOTBINDIR)/$(BMEMRUN_EXE)

#*--- bmem.css --------------------------------------------------------*/
css: bmem.css
bmem.css: $(BOOTLIBDIR)/bmem/bmem.css
$(BOOTLIBDIR)/bmem/bmem.css: $(BOOTLIBDIR)/bmem
	cp etc/bmem.css $(BOOTLIBDIR)/bmem/bmem.css

#*--- bmem ------------------------------------------------------------*/
bmem:
	if [ ! -f .afile ]; then \
	  $(MAKE) .afile || exit 1; \
	fi
	$(MAKE) $(BOOTBINDIR)/$(BMEM_EXE)

$(BOOTBINDIR)/$(BMEM_EXE): $(OBJECT)
	$(BIGLOO) $(BMEMFLAGS) -o $@ $^

#*--- lib -------------------------------------------------------------*/
lib: $(BOOTLIBDIR)/bmem/bmem_s.$(SHAREDSUFFIX) \
  $(BOOTLIBDIR)/bmem/bmem_u.$(SHAREDSUFFIX) \
  $(BOOTLIBDIR)/bmem/bmem_fth_s.$(SHAREDSUFFIX) \
  $(BOOTLIBDIR)/bmem/bmem_fth_u.$(SHAREDSUFFIX)

$(BOOTLIBDIR)/bmem/bmem_u.$(SHAREDSUFFIX): $(LIBOBJECT) $(BOOTLIBDIR)/bmem
	$(MAKE) shared-lib \
	   OBJECTS="$(LIBOBJECT)" \
           FORCELD=true \
           LIBDEST=$(BOOTLIBDIR)/bmem/bmem_u.$(SHAREDSUFFIX) \
	   LIBPATH=$(BOOTLIBDIR) \
           SONAME=bmem_u.$(SHAREDSUFFIX) \
           CLOSELIBS="-lbigloo_u-$(RELEASE)"

$(BOOTLIBDIR)/bmem/bmem_s.$(SHAREDSUFFIX): $(LIBOBJECT) $(BOOTLIBDIR)/bmem
	$(MAKE) shared-lib \
	   OBJECTS="$(LIBOBJECT)" \
           FORCELD=true \
           LIBDEST=$(BOOTLIBDIR)/bmem/bmem_s.$(SHAREDSUFFIX) \
	   LIBPATH=$(BOOTLIBDIR) \
           SONAME=bmem_s.$(SHAREDSUFFIX) \
           CLOSELIBS="-lbigloo_s-$(RELEASE)"

$(BOOTLIBDIR)/bmem/bmem_fth_u.$(SHAREDSUFFIX): $(LIBOBJECT) $(BOOTLIBDIR)/bmem
	$(MAKE) shared-lib \
	   OBJECTS="$(LIBOBJECT)" \
           FORCELD=true \
           LIBDEST=$(BOOTLIBDIR)/bmem/bmem_fth_u.$(SHAREDSUFFIX) \
	   LIBPATH=$(BOOTLIBDIR) \
           SONAME=bmem_fth_u.$(SHAREDSUFFIX) \
           CLOSELIBS="-lbigloo_u-$(RELEASE)"

$(BOOTLIBDIR)/bmem/bmem_fth_s.$(SHAREDSUFFIX): $(LIBOBJECT) $(BOOTLIBDIR)/bmem
	$(MAKE) shared-lib \
	   OBJECTS="$(LIBOBJECT)" \
           FORCELD=true \
           LIBDEST=$(BOOTLIBDIR)/bmem/bmem_fth_s.$(SHAREDSUFFIX) \
	   LIBPATH=$(BOOTLIBDIR) \
           SONAME=bmem_fth_s.$(SHAREDSUFFIX) \
           CLOSELIBS="-lbigloo_s-$(RELEASE)"

#*--- lib/bmem --------------------------------------------------------*/
$(BOOTLIBDIR)/bmem:
	mkdir -p $(BOOTLIBDIR)/bmem

#*---------------------------------------------------------------------*/
#*    Pop                                                              */
#*---------------------------------------------------------------------*/
pop:
	@ echo $(POPULATION:%=bde/bmem/%)
	@ (cd example; $(MAKE) pop)

#*---------------------------------------------------------------------*/
#*    install & uninstall                                              */
#*---------------------------------------------------------------------*/
install:
	if [ $(BINDIR) != $(BOOTBINDIR) ]; then \
	  cp $(BOOTBINDIR)/$(BMEM_EXE) $(DESTDIR)$(BINDIR)/$(BMEM_EXE) || exit 1; \
          chmod $(MODEXE) $(DESTDIR)$(BINDIR)/$(BMEM_EXE); \
	  cp $(BOOTBINDIR)/$(BMEMRUN_EXE) $(DESTDIR)$(BINDIR)/$(BMEMRUN_EXE) || exit 1; \
          chmod $(MODEXE) $(DESTDIR)$(BINDIR)/$(BMEMRUN_EXE); \
        fi
	if [ $(LIBDIR)/$(FILDIR) != $(BOOTLIBDIR) ]; then \
	  if [ ! -d $(LIBDIR)/$(FILDIR)/bmem ]; then \
            mkdir -p $(LIBDIR)/$(FILDIR)/bmem && \
              chmod $(MODDIR) $(LIBDIR)/$(FILDIR)/bmem; \
          fi && \
	  cp $(BOOTLIBDIR)/bmem/bmem_s.$(SHAREDSUFFIX) \
             $(LIBDIR)/$(FILDIR)/bmem/bmem_s.$(SHAREDSUFFIX) || exit 1; \
          chmod $(MODEXE) $(LIBDIR)/$(FILDIR)/bmem/bmem_s.$(SHAREDSUFFIX); \
	  cp $(BOOTLIBDIR)/bmem/bmem_u.$(SHAREDSUFFIX) \
             $(LIBDIR)/$(FILDIR)/bmem/bmem_u.$(SHAREDSUFFIX) || exit 1; \
          chmod $(MODEXE) $(LIBDIR)/$(FILDIR)/bmem/bmem_u.$(SHAREDSUFFIX); \
	  cp $(BOOTLIBDIR)/bmem/bmem_fth_s.$(SHAREDSUFFIX) \
             $(LIBDIR)/$(FILDIR)/bmem/bmem_fth_s.$(SHAREDSUFFIX) || exit 1; \
          chmod $(MODEXE) $(LIBDIR)/$(FILDIR)/bmem/bmem_fth_s.$(SHAREDSUFFIX); \
	  cp $(BOOTLIBDIR)/bmem/bmem_fth_u.$(SHAREDSUFFIX) \
             $(LIBDIR)/$(FILDIR)/bmem/bmem_fth_u.$(SHAREDSUFFIX) || exit 1; \
          chmod $(MODEXE) $(LIBDIR)/$(FILDIR)/bmem/bmem_fth_u.$(SHAREDSUFFIX); \
        fi
	cp etc/bmem.css $(LIBDIR)/$(FILDIR)/bmem/bmem.css
	chmod $(MODFILE) $(LIBDIR)/$(FILDIR)/bmem/bmem.css
	cp lib/bmem.h $(LIBDIR)/$(FILDIR)/bmem/bmem.h
	chmod $(MODFILE) $(LIBDIR)/$(FILDIR)/bmem/bmem.h
	cp lib/esymbol.h $(LIBDIR)/$(FILDIR)/bmem/esymbol.h
	chmod $(MODFILE) $(LIBDIR)/$(FILDIR)/bmem/esymbol.h

uninstall:
	- $(RM) -f $(DESTDIR)$(BINDIR)/$(BMEM_EXE)
	- $(RM) -f $(DESTDIR)$(BINDIR)/$(BMEMRUN_EXE)
	- $(RM) -rf $(LIBDIR)/$(FILDIR)/bmem

#*---------------------------------------------------------------------*/
#*    Clean                                                            */
#*---------------------------------------------------------------------*/
cleanbmem:
	- $(RM) -f .afile .etags
	- $(RM) -f $(OBJECT)
	- $(RM) -f $(BOOTBINDIR)/$(BMEM_EXE)

cleanlib:
	- $(RM) -f $(LIBOBJECT)
	- $(RM) -f $(BOOTLIBDIR)/bmem/bmem.$(SHAREDSUFFIX)

clean: cleanbmem cleanlib
	- $(RM) -f $(BOOTBINDIR)/$(BMEMRUN_EXE)
	- $(RM) -rf $(BOOTLIBDIR)/bmem

#*---------------------------------------------------------------------*/
#*    Automatic rule                                                   */
#*---------------------------------------------------------------------*/
lib/%.o: lib/%.c $(LIBINCLUDE:%=lib/%)
	$(CC) $(CEFLAGS) $(CPICFLAGS) $(CFLAGS) $(CMEMFLAGS) $(IFLAGS) $< -o $@ -c

bmem/%.o: bmem/%.scm $(INCLUDE:%=bmem/%)
	$(BIGLOO) $(EFLAGS) $(BMEMFLAGS) $< -o $@ -c

#*---------------------------------------------------------------------*/
#*    ude                                                              */
#*---------------------------------------------------------------------*/
.PHONY: ude .etags .afile

ude:
	@ $(MAKE) -f Makefile .afile .etags

.afile:
	@ $(BGLAFILE) -o .afile $(SRC:%=bmem/%)

.etags:
	@ $(BGLTAGS) -o .etags $(SRC:%=bmem/%)

dep:
	@(num=`grep -n '^#bdepend start' Makefile | awk -F: '{ print $$1}' -`;\
          head -n `expr $$num - 1` Makefile > /tmp/Makefile.aux)
	@ $(BGLDEPEND) -search-path . -fno-mco $(SRC:%=bmem/%) >> /tmp/Makefile.aux
	@ mv /tmp/Makefile.aux Makefile

getbinary:
	@ echo "$(BOOTBINDIR)/$(BMEM_EXE)"

getsources:
	@ echo $(SRC:%=bmem/%)

