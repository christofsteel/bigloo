;*=====================================================================*/
;*    serrano/prgm/project/bigloo/api/patch/src/Misc/make_lib.scm      */
;*    -------------------------------------------------------------    */
;*    Author      :  Manuel Serrano                                    */
;*    Creation    :  Tue Nov  6 15:09:37 2001                          */
;*    Last change :  Mon Jun 26 19:11:45 2017 (serrano)                */
;*    Copyright   :  2001-17 Manuel Serrano                            */
;*    -------------------------------------------------------------    */
;*    The module used to build the heap file.                          */
;*=====================================================================*/

;*---------------------------------------------------------------------*/
;*    The module                                                       */
;*---------------------------------------------------------------------*/
(module __patch_makelib

   (import  __patch_patch)

   (eval    (export-all)
            (class PatchDescr)))

