/*=====================================================================*/
/*    serrano/prgm/project/bigloo/api/mpg123/src/Clib/bglmpg123.c      */
/*    -------------------------------------------------------------    */
/*    Author      :  Manuel Serrano                                    */
/*    Creation    :  Mon Jun 20 14:50:56 2011                          */
/*    Last change :  Tue Jun 28 08:55:37 2011 (serrano)                */
/*    Copyright   :  2011 Manuel Serrano                               */
/*    -------------------------------------------------------------    */
/*    mpg123 Bigloo binding                                            */
/*=====================================================================*/
#include <mpg123.h>
#include <bigloo.h>
#include "bglmpg123.h"

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_mpg123_decoders ...                                          */
/*---------------------------------------------------------------------*/
obj_t bgl_mpg123_decoders() {
   const char **a = mpg123_decoders();
   obj_t l = BNIL;

   while( *a ) l = MAKE_PAIR( string_to_bstring( (char *)*a++ ), l );

   return l;
}
			      
/*---------------------------------------------------------------------*/
/*    mpg123_handle *                                                  */
/*    bgl_mpg123_new ...                                               */
/*---------------------------------------------------------------------*/
mpg123_handle *
bgl_mpg123_new( const char *decoder ) {
   int err;
   mpg123_handle *m = mpg123_new( decoder, &err );

   if( !m ) {
      bgl_mpg123_error(
	 "bgl-mpg123-new",
	 mpg123_plain_strerror( err ),
	 string_to_bstring( !decoder ? "default" : (char *)decoder ) );
   }

   return m;
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_mpg123_decode ...                                            */
/*---------------------------------------------------------------------*/
int
bgl_mpg123_decode( mpg123_handle *m, char *inbuf, long insz, char *outbuf, long outsz ) {
   size_t size;
   int ret;
   obj_t env = BGL_CURRENT_DYNAMIC_ENV();
   
   ret = mpg123_decode( m, inbuf, insz, outbuf, outsz, &size );

   BGL_ENV_MVALUES_NUMBER_SET( env, 2 );
   BGL_ENV_MVALUES_VAL_SET( env, 1, BINT( size ) );
   
   return ret;
}

/*---------------------------------------------------------------------*/
/*    long                                                             */
/*    bgl_mpg123_getformat ...                                         */
/*---------------------------------------------------------------------*/
long
bgl_mpg123_getformat( mpg123_handle *m ) {
   long rate;
   int channels, encoding;
   obj_t env = BGL_CURRENT_DYNAMIC_ENV();
   
   mpg123_getformat( m, &rate, &channels, &encoding );
   
   BGL_ENV_MVALUES_NUMBER_SET( env, 3 );
   BGL_ENV_MVALUES_VAL_SET( env, 1, BINT( channels ) );
   BGL_ENV_MVALUES_VAL_SET( env, 2, BINT( encoding ) );
   
   return rate;
}

/*---------------------------------------------------------------------*/
/*    long                                                             */
/*    bgl_mpg123_position ...                                          */
/*---------------------------------------------------------------------*/
long
bgl_mpg123_position( mpg123_handle *m ) {
   off_t frame = mpg123_tellframe( m );
   double tpf = mpg123_tpf( m );

   return (long)(tpf * 1000.) * frame;
/*    off_t current_frame, frames_left;                                */
/*    double current_seconds, seconds_left;                            */
/*    long cs, sl;                                                     */
/*                                                                     */
/*    mpg123_position( m,                                              */
/* 		    0, 0,                                              */
/* 		    &current_frame, &frames_left,                      */
/* 		    &current_seconds, &seconds_left );                 */
/*                                                                     */
/*    return (long)(current_seconds * 1000.);                          */
}

/*---------------------------------------------------------------------*/
/*    long                                                             */
/*    bgl_mpg123_info ...                                              */
/*---------------------------------------------------------------------*/
long
bgl_mpg123_info( mpg123_handle *m ) {
   struct mpg123_frameinfo mi;
   int err = mpg123_info( m, &mi );
   obj_t env = BGL_CURRENT_DYNAMIC_ENV();

   if( err < 0 ) {
      bgl_mpg123_error( "mpg123-info", mpg123_plain_strerror( err ), m );
   }

   BGL_ENV_MVALUES_NUMBER_SET( env, 1 );
   BGL_ENV_MVALUES_VAL_SET( env, 1, BINT( mi.rate ) );

   return mi.bitrate;
}

/*---------------------------------------------------------------------*/
/*    long                                                             */
/*    bgl_mpg123_getvolume ...                                         */
/*---------------------------------------------------------------------*/
long
bgl_mpg123_getvolume( mpg123_handle *m ) {
   double base, really, rva_db;
   int err;

   err = mpg123_getvolume( m, &base, &really, &rva_db );

   if( err < 0 ) {
      return err;
   } else {
      fprintf( stderr, "vol base=%d really=%d rva_db=%d\n",
	       (int)(base * 100), (int)(really * 100), (int)(rva_db * 100) );
      return (long)(base * 100);
   }
}