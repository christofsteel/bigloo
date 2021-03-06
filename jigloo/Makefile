#*=====================================================================*/
#*    serrano/prgm/project/bigloo/jigloo/Makefile                      */
#*    -------------------------------------------------------------    */
#*    Author      :  Manuel Serrano                                    */
#*    Creation    :  Wed Jan 14 14:46:31 1998                          */
#*    Last change :  Sun Jul  9 10:26:28 2017 (serrano)                */
#*    Copyright   :  1998-2017 Manuel Serrano, see LICENSE file        */
#*    -------------------------------------------------------------    */
#*    The Makefile to build Jigloo. This makefile has to be processed  */
#*    either by Gnu make or Sun make.                                  */
#*=====================================================================*/

#*---------------------------------------------------------------------*/
#*    Default configuration                                            */
#*---------------------------------------------------------------------*/
include ../Makefile.buildconfig
include ../Makefile.config

#*---------------------------------------------------------------------*/
#*    Compilers, Tools and Destinations                                */
#*---------------------------------------------------------------------*/
# the shell to be used
SHELL           = /bin/sh
# the produced executable
JIGLOO_DEST	= $(BOOTBINDIR)/jigloo.class

#*---------------------------------------------------------------------*/
#*    The source                                                       */
#*---------------------------------------------------------------------*/
SOURCE		= jigloo.java
OBJECTS		= jigloo.class

#*---------------------------------------------------------------------*/
#*    Tout le code ...                                                 */
#*---------------------------------------------------------------------*/
O_OBJECTS       = $(OBJECTS)

POPULATION	= $(SOURCE) README Makefile

#*---------------------------------------------------------------------*/
#*     Les suffixes ...                                                */
#*---------------------------------------------------------------------*/
.SUFFIXES:
.SUFFIXES: .java .class

#*---------------------------------------------------------------------*/
#*    L'executable `jigloo'                                            */
#*---------------------------------------------------------------------*/
jvm: jigloo
jigloo: 
	@ $(MAKE) $(JIGLOO_DEST)

#*---------------------------------------------------------------------*/
#*    $(JIGLOO_DEST)                                                   */
#*---------------------------------------------------------------------*/
$(JIGLOO_DEST): $(O_OBJECTS)
	@ mv jigloo.class $@
	@ echo "Jigloo Done..."
	@ echo "-------------------------------"

#*---------------------------------------------------------------------*/
#*    install                                                          */
#*---------------------------------------------------------------------*/
.PHONY: install uninstall

install:
	if [ $(DESTDIR)$(BINDIR) != $(BOOTBINDIR) ]; then \
	   cp $(BOOTBINDIR)/jigloo.class $(DESTDIR)$(BINDIR); \
	   chmod $(MODFILE) $(DESTDIR)$(BINDIR)/jigloo.class; \
        fi

uninstall:
	- $(RM) -f $(DESTDIR)$(BINDIR)/jigloo.class

#*---------------------------------------------------------------------*/
#*    tags                                                             */
#*---------------------------------------------------------------------*/
tags:
	@ etags -o .etags $(SOURCE) 

#*---------------------------------------------------------------------*/
#*    distrib                                                          */
#*    -------------------------------------------------------------    */
#*    We prepare the compiler for a distribution                       */
#*---------------------------------------------------------------------*/
distrib: 
	@ if [ `pwd` = $(BOODIR)/jigloo ]; then \
             echo "*** ERROR:Illegal dir to make a distrib `pwd`"; \
             exit 1; \
          fi
	@ $(MAKE) cleanall

#*---------------------------------------------------------------------*/
#*     clean ...                                                       */
#*---------------------------------------------------------------------*/
.PHONY: clean cleanall distclean

clean:
	@- $(RM) -f $(O_OBJECTS)
	@ find . \( -name '*[~%]' \
                       -o -name '.??*[~%]' \
                       -o -name '#*#' \
                       -o -name '?*#' \
                       -o -name \*core \) \
                     -type f -exec rm {} \;   
	@ echo "cleanup done..."
	@ echo "-------------------------------"

cleanall: clean
	@- $(RM) -f $(JIGLOO_DEST)

distclean: cleanall

#*---------------------------------------------------------------------*/
#*     touchall ...                                                    */
#*---------------------------------------------------------------------*/
touchall:
	@ touch $(SOURCE)
	@ echo "touchall done..."
	@ echo "-------------------------------"

#*---------------------------------------------------------------------*/
#*    pop                                                              */
#*    -------------------------------------------------------------    */
#*    This entry is used by the bigloo/Makefile (the main Bigloo       */
#*    makefile) to get the list of file that populate a revision.      */
#*---------------------------------------------------------------------*/
.PHONY: pop
pop:
	@ echo $(POPULATION:%=jigloo/%)

#*---------------------------------------------------------------------*/
#*     .java --> .class                                                */
#*---------------------------------------------------------------------*/
.java.class:
	@ $(JAVAC) $*.java
