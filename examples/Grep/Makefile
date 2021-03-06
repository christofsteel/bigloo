#*=====================================================================*/
#*    serrano/prgm/project/bigloo/examples/Grep/Makefile               */
#*    -------------------------------------------------------------    */
#*    Author      :  Manuel Serrano                                    */
#*    Creation    :  Tue Jan 30 15:19:19 1996                          */
#*    Last change :  Sun Jul  9 10:23:31 2017 (serrano)                */
#*    Copyright   :  1996-2017 Manuel Serrano, see LICENSE file        */
#*    -------------------------------------------------------------    */
#*    The makefile to build grep                                       */
#*=====================================================================*/

#*---------------------------------------------------------------------*/
#*    The default configuration                                        */
#*---------------------------------------------------------------------*/
include ../../Makefile.buildconfig
include ../../Makefile.config

#*---------------------------------------------------------------------*/
#*    flags                                                            */
#*---------------------------------------------------------------------*/
BIGLOO          = $(BOOTBINDIR)/bigloo
BGLOPTFLAGS	= -O +rm
BFLAGS		= $(BGLOPTFLAGS)
EFLAGS		=
DEST		= grep$(EXE_SUFFIX)

#*---------------------------------------------------------------------*/
#*    Objects and sources                                              */
#*---------------------------------------------------------------------*/

#*--- scm -------------------------------------------------------------*/
SCM_FILE	= grep

SCM_OBJ		= $(SCM_FILE:%=%.o)
SCM_SRC		= $(SCM_OBJ:%.o=%.scm)

#*---------------------------------------------------------------------*/
#*    All objects and sources                                          */
#*---------------------------------------------------------------------*/
OBJ		= $(C_OBJ) $(SCM_OBJ)
SRC		= $(C_SRC) $(SCM_SRC)

POPULATION	= $(SRC:%=examples/Grep/%) examples/Grep/README examples/Grep/Makefile

#*---------------------------------------------------------------------*/
#*    the goals.                                                       */
#*---------------------------------------------------------------------*/
$(DEST): $(OBJ) 
	$(BIGLOO) $(LNFLAGS) $(OBJ) -o $(DEST)

test: $(DEST)
	./$(DEST)

pop:
	@ echo $(POPULATION)

#*---------------------------------------------------------------------*/
#*    Cleaning                                                         */
#*---------------------------------------------------------------------*/
.PHONY: clean

clean:
	@ find . \( -name '*[~%]' \
                       -o -name '.??*[~%]' \
                       -o -name '#*#' \
                       -o -name '?*#' \
                       -o -name \*core \) \
                     -type f -exec $(RM) {} \;
	@- $(RM) -f $(OBJ)
	@- $(RM) -f $(DEST)

#*---------------------------------------------------------------------*/
#*    Suffixes                                                         */
#*---------------------------------------------------------------------*/
.SUFFIXES:
.SUFFIXES: .scm .c .o

#*---------------------------------------------------------------------*/
#*    .c.o                                                             */
#*---------------------------------------------------------------------*/
.c.o:
	@ echo $*.c:
	@ $(CC) -c $(CFLAGS) $*.c -o $*.o

#*---------------------------------------------------------------------*/
#*    .scm.o                                                           */
#*---------------------------------------------------------------------*/
.scm.o:
	$(BIGLOO) -c $(BFLAGS) $(EFLAGS) $*.scm -o $*.o
