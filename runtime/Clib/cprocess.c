/*=====================================================================*/
/*    serrano/prgm/project/bigloo/runtime/Clib/cprocess.c              */
/*    -------------------------------------------------------------    */
/*    Author      :  Erick Gallesio                                    */
/*    Creation    :  Mon Jan 19 17:35:12 1998                          */
/*    Last change :  Thu Jan  7 20:04:51 2010 (serrano)                */
/*    -------------------------------------------------------------    */
/*    Process handling C part. This part is mostly compatible with     */
/*    STK. This code is extracted from STK by Erick Gallesio.          */
/*=====================================================================*/
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#if defined( _MINGW_VER )
#   define _BGL_WIN32_VER
#   include <io.h>
#   include <process.h>
#   include <windows.h>
#   include <stdlib.h>
typedef int intptr_t;
#else
#  if !defined( _MSC_VER )
#    include <sys/param.h>
#    include <sys/wait.h>
#    include <unistd.h>
#  else
#    define _BGL_WIN32_VER
#    include <io.h>
#    include <process.h>
#    include <windows.h>
#  endif
#endif
#include <bigloo.h>

#define MSG_SIZE 1024

#if( HAVE_SIGCHLD )
#   define PURGE_PROCESS_TABLE()	/* Nothing to do */
#else
#   define PURGE_PROCESS_TABLE() process_terminate_handler( 0 )
#endif

#ifdef _BGL_WIN32_VER
#  define close _close
#  define dup _dup
#  define execve _execve
#  define execvp _execvp
#  define pipe( fd_array ) _pipe( fd_array, 1024, _O_TEXT )
#endif

#ifndef NOFILE
#  define NOFILE 256
#endif

/*---------------------------------------------------------------------*/
/*    Imports                                                          */
/*---------------------------------------------------------------------*/
extern obj_t bgl_make_input_port( obj_t, FILE *, obj_t, obj_t );
extern long default_io_bufsiz;
extern obj_t bgl_close_input_port( obj_t );
extern long  bgl_list_length( obj_t );
extern char *bgl_string_to_gc_cstring( obj_t );
extern obj_t string_to_bstring( char * );

/*---------------------------------------------------------------------*/
/*    Prototypes                                                       */
/*---------------------------------------------------------------------*/
#ifdef _BGL_WIN32_VER
BGL_RUNTIME_DECL char *bgl_get_last_error_message( char * );
#endif

static void process_terminate_handler( int );

/*---------------------------------------------------------------------*/
/*    Process mutex                                                    */
/*---------------------------------------------------------------------*/
static obj_t process_mutex = BUNSPEC;
DEFINE_STRING( process_mutex_name, _1, "process-mutex", 13 );

/*---------------------------------------------------------------------*/
/*    process table                                                    */
/*---------------------------------------------------------------------*/
#define DEFAULT_MAX_PROC_NUM 255
static int max_proc_num = 0;               /* (simultaneous processes) */
static obj_t *proc_arr;                    /* process table            */

static char *std_streams[ 3 ] = {
  "input",	
  "output",	
  "error",
};

/*---------------------------------------------------------------------*/
/*    void                                                             */
/*    bgl_init_process_table ...                                       */
/*---------------------------------------------------------------------*/
void
bgl_init_process_table() {
   int i;
   char *env;
   
   process_mutex = bgl_make_mutex( process_mutex_name );

   env = getenv( "BIGLOOLIVEPROCESS" );
   
   if( env ) {
      max_proc_num = atoi( env );
      if( max_proc_num < 0 ) max_proc_num = DEFAULT_MAX_PROC_NUM;
   } else {
      max_proc_num = DEFAULT_MAX_PROC_NUM;
   }
   proc_arr = (obj_t *)GC_MALLOC( (max_proc_num + 1) * sizeof( obj_t ) );

   /* we first initialize the process table */
   for( i = 0; i < max_proc_num; i++ ) proc_arr[ i ] = BUNSPEC;

#if HAVE_SIGCHLD
   /* On systems which support SIGCHLD, the processes table is cleaned */
   /* up as soon as a process terminate. On other systems this is done */
   /* from time to time to avoid filling the table too fast.           */

# if HAVE_SIGACTION
  {
    /* Use the secure Posix.1 way */
    struct sigaction sigact;
    
    sigemptyset( &(sigact.sa_mask) );
    sigact.sa_handler = process_terminate_handler;
    /* Ignore SIGCHLD generated by SIGSTOP */
    sigact.sa_flags   = SA_NOCLDSTOP;     
#  if( defined( SA_RESTART ) )
    /* Thanks to Harvey J. Stein <hjstein@MATH.HUJI.AC.IL> for the fix */
    sigact.sa_flags  |= SA_RESTART;
#  endif
    sigaction( SIGCHLD, &sigact, NULL );
  }
# else
  /* Use "classical" way. (Only Solaris 2 seems to have problem with it */
  signal( SIGCHLD, process_terminate_handler );
# endif
#endif
}

/*---------------------------------------------------------------------*/
/*    bool_t                                                           */
/*    c_process_alivep ...                                             */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
bool_t
c_process_alivep( obj_t process )
#ifndef _BGL_WIN32_VER
{
   if( PROCESS( process ).exited || (PROCESS( process ).pid == 0) )
      return 0;
   else {
      int info, res;

      /* Use waitpid to gain the info. */
      res = waitpid( PROCESS_PID( process ), &info, WNOHANG );
      
      if( res == 0 ) 
	 /* process is still running */
	 return 1;
      else
      {
	 if( res == PROCESS_PID( process ) )
	 {
	    /* process has terminated and we must save this information */
	    PROCESS(process).exited      = 1;
	    PROCESS(process).exit_status = info;
	    return 0;
	 }
	 else
	    return 0;
      }
   }
}
#else
{
   if( PROCESS( process ).exited || (PROCESS( process ).pid == 0) )
      return 0;
   else
   {
      DWORD exit_code;

      if( !GetExitCodeProcess( PROCESS( process ).hProcess, &exit_code ) )
      {
         C_SYSTEM_FAILURE( BGL_PROCESS_EXCEPTION,
                           "process-alive?",
                           bgl_get_last_error_message( "Could neither determine process exit code nor get the error message." ),
                           BFALSE );
      }

      if( exit_code == STILL_ACTIVE )
         /* process is still running */
         return 1;

      /* process is now terminated */
      PROCESS( process ).exited      = 1;
      PROCESS( process ).exit_status = exit_code;
      CloseHandle( PROCESS( process ).hProcess );
      PROCESS( process ).hProcess= INVALID_HANDLE_VALUE;
      return 0;
   }
}
#endif

/*---------------------------------------------------------------------*/
/*    void                                                             */
/*    c_unregister_process ...                                         */
/*    -------------------------------------------------------------    */
/*    When this function is called, the process_mutex is already       */
/*    acquired.                                                        */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
void
c_unregister_process( obj_t proc ) {
   int i;

   for( i = 0; i < 3; i++ ) {
      obj_t p = PROCESS( proc ).stream[ i ];

      if( INPUT_PORTP( p ) && (PORT( p ).kindof != KINDOF_PROCPIPE) )
	 bgl_close_input_port( p );

      if( OUTPUT_PORTP( p ) && (PORT( p ).kindof != KINDOF_PROCPIPE) )
	 bgl_close_output_port( p );
   }

#ifdef _BGL_WIN32_VER
   if (PROCESS( proc ).hProcess != INVALID_HANDLE_VALUE)
   {
     CloseHandle( PROCESS( proc ).hProcess );
     PROCESS( proc ).hProcess= INVALID_HANDLE_VALUE;
   }
#endif

   proc_arr[ PROCESS( proc ).index ] = BUNSPEC;
}
   
/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    process_terminate_handler ...                                    */
/*---------------------------------------------------------------------*/
static void
process_terminate_handler( int sig ) {
  register int i;
  obj_t proc;

#if( HAVE_SIGCHLD && !HAVE_SIGACTION )
  static int in_handler = 0;

  /* Necessary on System V */
  signal( SIGCHLD, process_terminate_handler ); 
  if( in_handler++ ) /* Execution is re-entrant */ return;
  
  do {
#endif
     /* Find the process which is terminated                          */
     /* Note that this loop can find:                                 */
     /*      - nobody: if the process has been destroyed by GC        */
     /*      - 1 process: This is the normal case                     */
     /*	    - more than one process: This can arise when:             */
     /*		- we use signal rather than sigaction                 */
     /*		- we don't have SIGCHLD and this function is called   */
     /*		  by PURGE_PROCESS_TABLE                              */
     /* Sometimes I think that life is a little bit complicated (ndrl */
     /* sic Erick Gallesio :-)                                        */
     bgl_mutex_lock( process_mutex );
     for( i = 0; i < max_proc_num; i++ ) {
	proc = proc_arr[ i ];

	if( PROCESSP( proc ) && (!c_process_alivep( proc )) ) {
	   /* This process has exited. We can delete it from the table*/
	   c_unregister_process( proc );
	}
     }
     bgl_mutex_unlock( process_mutex );

#if( HAVE_SIGCHLD && !HAVE_SIGACTION )
     /* Since we can be called recursively, we have perhaps forgot to */
     /* delete some dead process from the table. So, we have perhaps  */
     /* to scan the process array another time                        */
  } while ( --in_handler > 0 );
#endif
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    cannot_run ...                                                   */
/*---------------------------------------------------------------------*/
#ifndef _BGL_WIN32_VER
static void
cannot_run( int pipes[ 3 ][ 2 ], obj_t bcommand, char *msg ) {
   int i;

   for( i = 0; i < 3; i++ ) {
      if( pipes[ i ][ 0 ] != -1 ) close( pipes[ i ][ 0 ] );
      if( pipes[ i ][ 1 ] != -1 ) close( pipes[ i ][ 1 ]);
   }

   C_SYSTEM_FAILURE( BGL_PROCESS_EXCEPTION, "run-process", msg, bcommand );
}
#else
static void
cannot_run( HANDLE pipes[ 3 ][ 2 ], obj_t bcommand, char *msg ) {
   int i, j;

   for( i = 0; i < 3; i++ )
   {
      for( j = 0; j < 2; j++ )
      {
         if( pipes[ i ][ j ] != INVALID_HANDLE_VALUE )
         {
           CloseHandle( pipes[ i ][ j ] );
         }
      }
   }
  
   C_SYSTEM_FAILURE( BGL_PROCESS_EXCEPTION, "run-process", msg, bcommand );
}
#endif
 
/*---------------------------------------------------------------------*/
/*    static obj_t                                                     */
/*    make_process ...                                                 */
/*---------------------------------------------------------------------*/
static obj_t
make_process() {
   int   i;
   obj_t a_proc;

   PURGE_PROCESS_TABLE();

   a_proc = GC_MALLOC( PROCESS_SIZE );
   a_proc->process_t.header = MAKE_HEADER( PROCESS_TYPE, 0 );
   a_proc->process_t.stream[ 0 ] = BFALSE;
   a_proc->process_t.stream[ 1 ] = BFALSE;
   a_proc->process_t.stream[ 2 ] = BFALSE;
   a_proc->process_t.exit_status = 0;
   a_proc->process_t.exited = 0;

   /* Enter this process in the process table */
   bgl_mutex_lock( process_mutex );
   for( i = 0; (i < max_proc_num) && (proc_arr[ i ] != BUNSPEC); i++ );
   
   if( i == max_proc_num ) {
      bgl_mutex_unlock( process_mutex );
      C_SYSTEM_FAILURE( BGL_PROCESS_EXCEPTION,
			"make-process",
			"too many processes",
			BUNSPEC );
   }
   else {
      bgl_mutex_unlock( process_mutex );
      a_proc->process_t.index = i;
      proc_arr[ i ] = BREF( a_proc );
   }

   return BREF( a_proc );
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_process_nil ...                                              */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
bgl_process_nil() {
   static obj_t proc_nil = 0L;

   if( !proc_nil ) {
      proc_nil = make_process();
      bgl_mutex_lock( process_mutex );
      c_unregister_process( proc_nil );
      bgl_mutex_unlock( process_mutex );
   }
   return proc_nil;
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_run_process ...                                                */
/*---------------------------------------------------------------------*/
obj_t
c_run_process( obj_t bhost, obj_t bfork, obj_t bwaiting,
	       obj_t binput, obj_t boutput, obj_t berror,
	       obj_t bcommand, obj_t bargs, obj_t benv )
#ifndef _BGL_WIN32_VER
{
   /* Unix code */
   int pid = -1, i, argc;
   obj_t redirection[ 3 ];
   int pipes[ 3 ][ 2 ];
   char msg[ MSG_SIZE ], **argv, **argv_start;
   obj_t runner;
   obj_t proc;
   obj_t name;

   /* converting "null:" keywords to null file names */
   if( KEYWORDP( boutput ) &&
       (strcmp( BSTRING_TO_STRING( KEYWORD_TO_STRING( boutput ) ), "null:" ) == 0))
      boutput = string_to_bstring( "/dev/null" );
   if( KEYWORDP( berror ) &&
       (strcmp( BSTRING_TO_STRING( KEYWORD_TO_STRING( berror ) ), "null:" ) == 0))
      berror = string_to_bstring( "/dev/null" );

   /* redirection initializations */
   redirection[ 0 ] = binput;
   redirection[ 1 ] = boutput;
   redirection[ 2 ] = berror;
   
   for (i = 0; i < 3; i++) {
      pipes[ i ][ 0 ] = pipes[ i ][ 1 ] = -1;
   }

   /* First try to look if this redirection has not already done        */
   /* This can arise by doing                                           */
   /*     output: "out" error: "out"       which is correct             */
   /*     output: "out" input: "out"       which is obviously incorrect */
   for( i = 0; i < 3; i++ ) {
      if( STRINGP( redirection[ i ] ) ) {
	 /* redirection to a file */
	 int j;
	 char *ri = BSTRING_TO_STRING( redirection[ i ] );
	 
	 for( j = 0; j < i; j++ ) {
	    if( j != i && STRINGP( redirection[ j ] ) ) {
	       struct stat stat_i, stat_j;
	       char *rj = BSTRING_TO_STRING( redirection[ j ] );
	       /* Do a stat to see if we try to open the same file 2    */
	       /* times. If stat == -1 this is probably because file    */
	       /* doesn't exist yet.                                    */
	       if( stat( ri, &stat_i ) == -1 )
		  continue;
	       if( stat( rj, &stat_j ) == -1 )
		  continue;
		
	       if( stat_i.st_dev==stat_j.st_dev &&
		   stat_i.st_ino==stat_j.st_ino ) {
		  /* Same file was cited 2 times */
		  if( i == 0 || j == 0 ) {
		     sprintf( msg, "read/write on the same file: %s", ri );
		     cannot_run( pipes, bcommand, msg );
		  }

		  /* assert(i == 1 && j == 2 || i == 2 && j == 1); */
		  pipes[ i ][ 0 ] = dup( pipes[ j ][ 0 ] );

		  if( pipes[ i ][ 0 ] == -1 )
		     printf( "ERROR: %s", strerror( errno ) );
		  break;
	       }
	    }
	 }
	    
	 /* Two cases are possible here:                                     */
	 /* - we have stdout and stderr redirected on the same file (j != 3) */
	 /* - we have not found current file in list of redirections (j == 3)*/
	 if( j == i ) {
	    pipes[ i ][ 0 ] = open( ri,
				    i==0 ? O_RDONLY:(O_WRONLY|O_CREAT|O_TRUNC),
				    0666 );
	 }
	    
	 if( pipes[ i ][ 0 ] < 0 ) {
	    sprintf( msg,
		     "can't redirect standard %s to file %s",
		     std_streams[ i ],
		     ri );
	    cannot_run( pipes, bcommand, msg );
	 }
      } else {
	 if( KEYWORDP( redirection[ i ] ) ) {
	    /* redirection in a pipe */
	    if( pipe( pipes[ i ] ) < 0 ) {
	       sprintf( msg,
			"can't create stream for standard %s",
			std_streams[ i ] );

	       cannot_run( pipes, bcommand, msg );
	    }
	 }
      }
   }

   /* command + arguments initializations    */
   /* 4 = rsh + host + command + args + null */
   argc = 0;
   argv_start =
      (char **)GC_MALLOC_ATOMIC((bgl_list_length(bargs) + 4) * sizeof(char *));
   argv = argv_start + 2;

   argv[ argc++ ] = BSTRING_TO_STRING( bcommand );
   for( runner = bargs; PAIRP( runner ); runner = CDR( runner ) )
      argv[ argc++ ] = BSTRING_TO_STRING( CAR( runner ) );
   argv[ argc ] = 0L;
   
   /* rsh initialization */
   if( STRINGP( bhost ) ) {
      argc += 2;
      argv = argv_start;
      argv[ 0 ] = "rsh";
      argv[ 1 ] = BSTRING_TO_STRING( bhost );
   }

   /* proc object creation */
   proc = make_process();

   switch( CBOOL( bfork ) && (pid = fork()) ) {
      case 0:
	 /* The child process */
	 for( i = 0; i < 3; i++ ) {
	    if( STRINGP( redirection[ i ] ) ) {
	       /* redirection in a file */
	       close( i );
	       dup( pipes[ i ][ 0 ] );
	       close( pipes[ i ][ 0 ] );
	    } else {
	       if( KEYWORDP( redirection[ i ] ) ) {
		  /* redirection in a pipe */
		  close( i );
		  dup( pipes[ i ][ i == 0 ? 0 : 1 ] );
		  close( pipes[ i ][ 0 ] );
		  close( pipes[ i ][ 1 ] );
	       }
	    }
	 }

	 for( i = 3; i < NOFILE; i++ ) close( i );

	 /* and now we do the exec */
	 if( PAIRP( benv ) ) {
	    extern int bgl_envp_len;
	    extern char **bgl_envp;
	    int len = bgl_envp_len + bgl_list_length( benv );
	    char **envp, **crunner, **init_envp;

	    crunner = envp = (char **)alloca( sizeof( char * ) * (len + 1) );

	    if( bgl_envp ) {
	       for( init_envp = bgl_envp;
		    *init_envp;
		    init_envp++, crunner++ ) {
		  *crunner = *init_envp;
	       }
	    }

	    for( runner = benv;
		 PAIRP( runner );
		 crunner++, runner = CDR( runner ) ) {
	       *crunner = BSTRING_TO_STRING( CAR( runner ) );
	    }
	    *crunner = 0;

	    execve( *argv, argv, envp );
	 } else {
	    execvp( *argv, argv );
	 }

	 /* Don't try to do anything here, no display, no nothing. Since we */
	 /* are in the child raising or signalling an error would be bad    */
	 exit( 1 );

      default:
	 if( pid == -1 ) {
	    sprintf( msg, "Can't create child process: %s", strerror(errno) );
	    cannot_run( pipes, bcommand, msg );
	    return proc;
	 }

	 /* This is the parent process */
	 PROCESS( proc ).pid = pid;
	 for( i = 0; i < 3; i++ ) {
	    if( STRINGP( redirection[ i ] ) ) {
	       /* redirection in a file */
	       close( pipes[ i ][ 0 ] );
	    } else {
	       if( KEYWORDP( redirection[ i ] ) ) {
		  /* redirection to a pipe */
		  FILE *f;

		  close( pipes[ i ][ i == 0 ? 0 : 1 ] );

		  /* make a new file descriptor to access the pipe */
		  f = ((i == 0) ?
		       fdopen( pipes[ i ][ 1 ], "w" ) :
		       fdopen( pipes[ i ][ 0 ], "r" ));

		  if( f == NULL )
		     cannot_run( pipes, bcommand, "cannot fdopen" );

		  sprintf( msg, "pipe-%s-%d", std_streams[ i ], pid );

		  /* port name must be heap allocated because it has */
		  /* the same lifetime as the created port.          */
		  name = string_to_bstring( msg );

		  if( i == 0 )
		     PROCESS( proc ).stream[ i ] =
			bgl_make_output_port( name, (void *)fileno( f ),
					      KINDOF_PROCPIPE,
					      make_string_sans_fill( 80 ),
					      (size_t (*)())write,
					      lseek,
					      close );
					      
		  else
		     PROCESS( proc ).stream[ i ] =
			bgl_make_input_port( name, f,
					     KINDOF_PROCPIPE,
					     make_string_sans_fill( default_io_bufsiz ) );
	       }
	    }
	 }
	 
	 if( CBOOL( bwaiting ) ) {
	    int info;
	    
	    if( waitpid( pid, &info, 0 ) != pid ) {
	       if( !PROCESS(proc).exited ) {
		  C_SYSTEM_FAILURE( BGL_PROCESS_EXCEPTION,
				    "run-process",
				    "illegal process termination",
				    bcommand );
	       }
	    } else {
	       PROCESS( proc ).exit_status = info;
	       PROCESS( proc ).exited = 1;
	    }
	 }
   }

   return proc;
}
#else 
{
  /* Win32 code */
  const int                  quote_command = (strchr( BSTRING_TO_STRING( bcommand ), '"' ) == NULL);
  size_t                     command_line_length = strlen( BSTRING_TO_STRING( bcommand ) )
                                                   + (quote_command ? 2 : 0);
  char *                     command_line;
  obj_t                      redirection[ 3 ];
  HANDLE                     pipes[ 3 ][ 2 ];
  char                       msg[ MSG_SIZE ];
  obj_t                      proc;
  SECURITY_ATTRIBUTES        inherited_sa= { sizeof( inherited_sa ), NULL, TRUE };
  char *                     environment= NULL;
  STARTUPINFO                startup_info;
  PROCESS_INFORMATION        process_information;
  BOOL                       process_created;
  HANDLE *                   stream_handles[ 3 ]= { &startup_info.hStdInput,
                                                    &startup_info.hStdOutput,
                                                    &startup_info.hStdError };
  obj_t                      runner;
  int                        i;

   /* converting "null:" keywords to null file names */
   if (   KEYWORDP( boutput )
        && (strcmp( BSTRING_TO_STRING( KEYWORD_TO_STRING( boutput ) ), "null:" ) == 0))
      boutput= string_to_bstring( "NUL:" );
   if (   KEYWORDP( berror )
       && (strcmp( BSTRING_TO_STRING( KEYWORD_TO_STRING( berror ) ), "null:" ) == 0))
      berror= string_to_bstring( "NUL:" );

  /* redirection initializations */
  redirection[ 0 ] = binput;
  redirection[ 1 ] = boutput;
  redirection[ 2 ] = berror;
  for( i = 0; i < 3; i++ )
    pipes[ i ][ 0 ] = pipes[ i ][ 1 ] = INVALID_HANDLE_VALUE;

  /* First try to look if this redirection has not already done       */
  /* This can arise by doing                                           */
  /*     output: "out" error: "out"       which is correct             */
  /*     output: "out" input: "out"       which is obviously incorrect */
  for( i = 0; i < 3; i++ )
  {
    if( STRINGP( redirection[ i ] ) )
    {
      /* redirection to a file */
      int                    j;
      char                   *ri = BSTRING_TO_STRING( redirection[ i ] );

      for( j = 0; j < i; j++ )
      {
        if( j != i && STRINGP( redirection[ j ] ) )
        {
          struct stat        stat_i, stat_j;
          char               *rj = BSTRING_TO_STRING( redirection[ j ] );

          /* Do a stat to see if we try to open the same file 2    */
          /* times. If stat == -1 this is probably because file    */
          /* doesn't exist yet.                                    */
          if( stat( ri, &stat_i ) == -1 )
            continue;
          if( stat( rj, &stat_j ) == -1 )
            continue;

          if(    (stat_i.st_dev==stat_j.st_dev)
              && (stat_i.st_ino==stat_j.st_ino) )
          {
            /* Same file was cited 2 times */
            if( i == 0 || j == 0 )
            {
              sprintf( msg, "read/write on the same file: %s", ri );
              cannot_run( pipes, bcommand, msg );
            }

            /* assert(i == 1 && j == 2 || i == 2 && j == 1); */
            if (!DuplicateHandle( GetCurrentProcess(),
                                  pipes[ j ][ 0 ],
                                  GetCurrentProcess(),
                                  &pipes[ i ][ 0 ],
                                  0,
                                  TRUE,
                                  DUPLICATE_SAME_ACCESS ))
            {
              sprintf( msg,
                       "can't reopen file: %s",
                       bgl_get_last_error_message( "unknown error" ) );
              cannot_run( pipes, bcommand, msg );
            }

            break;
          }
        }
      }

      /* Two cases are possible here:                                     */
      /* - we have stdout and stderr redirected on the same file (j != 3) */
      /* - we have not found current file in list of redirections (j == 3)*/
      if( j == i )
      {
        pipes[ i ][ 0 ] = CreateFile( ri,
                                      ((i == 0) ? GENERIC_READ : GENERIC_WRITE),
                                      FILE_SHARE_READ,
                                      &inherited_sa,
                                      ((i == 0) ? OPEN_EXISTING : CREATE_ALWAYS),
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL );
      }

      if( pipes[ i ][ 0 ] == INVALID_HANDLE_VALUE )
      {
        sprintf( msg,
                 "can't redirect standard %s to file %s: %s",
                 std_streams[ i ],
                 ri,
                 bgl_get_last_error_message( "unknown error" ) );
        cannot_run( pipes, bcommand, msg );
      }
    }
    else
    {
      if( KEYWORDP( redirection[ i ] ) )
      {
        /* redirection in a pipe */
        if (!CreatePipe( &pipes[ i ][ 0 ],
                         &pipes[ i ][ 1 ],
                         &inherited_sa,
                         0 ))
        {
          sprintf( msg,
                   "can't create stream for standard %s: %s",
                   std_streams[ i ],
                   bgl_get_last_error_message( "unknown error" ) );
          cannot_run( pipes, bcommand, msg );
        }
      }
    }
  }

  /* command + arguments initializations    */
  /* 4 = rsh + host + command + args + null */
  if( STRINGP( bhost ) )
    command_line_length += 4
                           + strlen( BSTRING_TO_STRING( bhost ) )
                           + 1;

  for( runner = bargs ; PAIRP( runner ) ; runner= CDR( runner ) )
  {
    const char *             current_arg;

    command_line_length += 3; // space + 2*double-quote

    for ( current_arg= BSTRING_TO_STRING( CAR( runner ) ) ;
          *current_arg != '\0' ;
          ++current_arg )
    {
      ++command_line_length;
      if (*current_arg == '"')
        ++command_line_length;
    }
  }

  command_line = (char*)alloca( command_line_length + 1 );

  if( STRINGP( bhost ) )
  {
    strcpy( command_line, "rsh " );
    strcat( command_line, BSTRING_TO_STRING( bhost ) );
    strcat( command_line, " " );
  }
  else
    command_line[ 0 ]= '\0';

  if( quote_command )
    strcat( command_line, "\"" );
  strcat( command_line, BSTRING_TO_STRING( bcommand ) );
  if( quote_command )
    strcat( command_line, "\"" );

  if (PAIRP( bargs ))
  {
    char *                   dest= command_line + strlen( command_line );

    for( runner= bargs ; PAIRP( runner ) ; runner= CDR( runner ) )
    {
      const char *           src;

      *dest++= ' ';
      *dest++= '"';
      for ( src= BSTRING_TO_STRING( CAR( runner ) ) ; *src != '\0' ; ++src )
      {
        if (*src == '"')
          *dest++= '\\';
        *dest++= *src;
      }
      *dest++= '"';
    }

    *dest= '\0';
  }

  if( PAIRP( benv ) )
  {
    extern const char * const *   bgl_envp;
    const char * const *          init_envp;
    const char *                  src;
    char *                        dest;
    size_t                        env_len= 0;

    if( bgl_envp )
      for( init_envp= bgl_envp ; *init_envp != '\0' ; ++init_envp )
        env_len+= (strlen( *init_envp ) + 1);
    for ( runner= benv ; PAIRP( runner ) ; runner= CDR( runner ) )
      env_len+= (strlen( BSTRING_TO_STRING( CAR( runner ) ) ) +1);

    dest= environment= (char *)alloca( (env_len + 1) * sizeof( char ) );

    if( bgl_envp )
      for( init_envp= bgl_envp ; *init_envp != '\0' ; ++init_envp )
      {
        src= *init_envp;
        while (*src != '\0')
          *dest++= *src++;
        *dest++= '\0';
      }

    for ( runner= benv ; PAIRP( runner ) ; runner= CDR( runner ) )
    {
      src= BSTRING_TO_STRING( CAR( runner ) );
      while (*src != '\0')
        *dest++= *src++;
      *dest++= '\0';
    }
    *dest++= '\0';

    assert( dest == environ+env_len );
  }

  ZeroMemory( &startup_info, sizeof( startup_info ) );
  startup_info.cb = sizeof( startup_info );
  startup_info.dwFlags = STARTF_USESTDHANDLES;

  ZeroMemory( &process_information, sizeof( process_information ) );

  for( i= 0 ; i < 3 ; ++i )
  {
    if( STRINGP( redirection[ i ] ) )
      /* redirection in a file */
      *(stream_handles[ i ])= pipes[ i ][ 0 ];
    else
      if( KEYWORDP( redirection[ i ] ) )
         *(stream_handles[ i ])= pipes[ i ][ i == 0 ? 0 : 1 ];
      else
        *(stream_handles[ i ])= GetStdHandle( (i == 0) ? STD_INPUT_HANDLE
                                                       : (i == 1) ? STD_OUTPUT_HANDLE
                                                                  : STD_ERROR_HANDLE );
  }

  process_created= CreateProcess( NULL, command_line, NULL, NULL, TRUE, 0, environment, NULL, &startup_info, &process_information );

  if( !process_created )
  {
    /* process creation failed */
    C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR,
          "run-process",
          bgl_get_last_error_message( "Could neither run the process nor get the error message." ),
          bcommand );
  }
  else
  {
    /* creation succeeded */
    obj_t proc = make_process();

    PROCESS_PID( proc ) = process_information.dwProcessId;
    PROCESS( proc ).hProcess = process_information.hProcess;
    CloseHandle( process_information.hThread );

    for( i= 0 ; i < 3 ; ++i )
    {
      if(KEYWORDP( redirection[ i ] ))
      {
        /* redirection in a pipe */
        int zb;
        FILE *f;
        obj_t name;

        CloseHandle( pipes[ i ][ i == 0 ? 0 : 1 ] );

        /* make a new file descriptor to access the pipe */
        zb = ((i == 0) ?
	      _open_osfhandle( (intptr_t)pipes[ i ][ 1 ], _O_APPEND )
	      : _open_osfhandle( (intptr_t)pipes[ i ][ 0 ], _O_RDONLY ));

        f = ((i == 0) ? fdopen( zb, "w" )
                      : fdopen( zb, "r"));

        if( f == NULL )
	{
          cannot_run( pipes, bcommand, "cannot fdopen" );
        }

        sprintf( msg, "pipe-%s-%d", std_streams[ i ], process_information.dwProcessId );

        /* port name must be heap allocated because it has */
        /* the same lifetime as the created port.          */
	name = string_to_bstring( msg );

        PROCESS( proc ).stream[ i ] =
	   (( i == 0 )
	    ? bgl_make_output_port( name, (void *)fileno( f ), KINDOF_PROCPIPE,
	                            make_string_sans_fill( 80 ),
				    (size_t (*)())write,
				    lseek,
				    close )
	    : bgl_make_input_port( name,
				   f,
				   KINDOF_PROCPIPE,
				   make_string_sans_fill( default_io_bufsiz ) ));
      }
    }

    if( CBOOL( bwaiting ) )
    {
      if( WaitForSingleObject( PROCESS( proc ).hProcess, INFINITE ) != WAIT_OBJECT_0 )
      {
        C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR,
                          "run-process",
                          bgl_get_last_error_message( "Could neither wait for process termination nor get the error message." ),
                          bcommand );
      }
      else
      {
          /* process is now terminated */
          DWORD exit_code;

          if( !GetExitCodeProcess( PROCESS( proc ).hProcess, &exit_code ) )
          {
            C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR,
                              "run-process",
                              bgl_get_last_error_message( "Could neither determine process exit code nor get the error message." ),
                              BFALSE );
          }

          PROCESS( proc ).exited      = 1;
          PROCESS( proc ).exit_status = exit_code;

          CloseHandle( PROCESS( proc ).hProcess );
          PROCESS( proc ).hProcess= INVALID_HANDLE_VALUE;
      }
    }

    return proc;
  }
}
#endif

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_list ...                                               */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_list() {
   int   i;
   obj_t lst = BNIL;

   PURGE_PROCESS_TABLE();

   for( i = 0; i < max_proc_num; i++ ) {
      obj_t p = proc_arr[ i ];
      if( PROCESSP( p ) && c_process_alivep( proc_arr[ i ] ) )
	 lst = MAKE_PAIR( p, lst );
   }
   
   return lst;
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_wait ...                                               */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_wait( obj_t proc )
#ifndef _BGL_WIN32_VER
{
   PURGE_PROCESS_TABLE();

   if( PROCESS( proc ).exited )
      return BFALSE;
   else {
      int ret = waitpid( PROCESS_PID(proc), &(PROCESS(proc).exit_status), 0 );

      PROCESS( proc ).exited = 1;
      return (ret == 0) ? BFALSE : BTRUE;
   }
}
#else
{
   PURGE_PROCESS_TABLE();

   if( PROCESS( proc ).exited )
      return BFALSE;
   else
   {
      obj_t result;

      if( WaitForSingleObject( PROCESS( proc ).hProcess, INFINITE ) != WAIT_OBJECT_0 )
         /* process is still running */
         result = BFALSE;
      else
      {
         /* process is now terminated */
         DWORD exit_code;

         if( !GetExitCodeProcess( PROCESS( proc ).hProcess, &exit_code ) )
         {
            C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR,
                              "process-wait",
                              bgl_get_last_error_message( "Could neither determine process exit code nor get the error message." ),
                              BFALSE );
         }

         PROCESS( proc ).exited = 1;
         PROCESS( proc ).exit_status = exit_code;
         CloseHandle( PROCESS( proc ).hProcess );
         PROCESS( proc ).hProcess= INVALID_HANDLE_VALUE;
         result = BTRUE;
      }
      return result;
   }
}
#endif

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_xstatus ...                                            */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_xstatus( obj_t proc )
#ifndef _BGL_WIN32_VER
{
   int info;

   PURGE_PROCESS_TABLE();

   if( PROCESS(proc).exited )
      info = PROCESS(proc).exit_status;
   else
   {
      if( waitpid( PROCESS_PID( proc ), &info, WNOHANG ) == 0 )
      {
	 /* process is still running */
	 return BFALSE;
      }
      else
      {
	 /* process is now terminated */
	 PROCESS( proc ).exited = 1;
	 PROCESS( proc ).exit_status = info;
      }
   }
   
   return BINT( WEXITSTATUS( info ) );
}
#else 
{
   PURGE_PROCESS_TABLE();

   if( PROCESS(proc).exited )
      return BINT( PROCESS(proc).exit_status );
   else
   {
      DWORD exit_code;

      if( !GetExitCodeProcess( PROCESS( proc ).hProcess, &exit_code ) )
	 C_SYSTEM_FAILURE( BGL_PROCESS_EXCEPTION,
			   "process-exit-status",
			   bgl_get_last_error_message( "Could neither determine process exit code nor get the error message." ),
			   BFALSE );

      if( exit_code == STILL_ACTIVE )
         /* process is still running */
         return BFALSE;

      /* process is now terminated */
      PROCESS( proc ).exited = 1;
      PROCESS( proc ).exit_status = exit_code;
      CloseHandle( PROCESS( proc ).hProcess );
      PROCESS( proc ).hProcess= INVALID_HANDLE_VALUE;
      return BINT( exit_code );
   }
}
#endif

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_send_signal ...                                        */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_send_signal( obj_t proc, int signal ) {
   PURGE_PROCESS_TABLE();
   
#ifndef WIN32
   kill( PROCESS_PID( proc ), signal );
#else
   /* Under Win32, signals can only be sent to the current process (cf. raise) */
#endif
   return BUNSPEC;
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_kill ...                                               */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_kill( obj_t proc ) {
#ifndef WIN32
#  if( defined( SIGTERM ) )
   return c_process_send_signal( proc, SIGTERM );
#  else
   return BUNSPEC;
#  endif
#else
   TerminateProcess( PROCESS( proc ).hProcess, 1 );
#endif
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_stop ...                                               */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_stop( obj_t proc ) {
#if( defined( SIGSTOP ) )
   return c_process_send_signal( proc, SIGSTOP );
#else
   return BUNSPEC;
#endif
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    c_process_continue ...                                           */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
c_process_continue( obj_t proc ) {
#if( defined( SIGCONT ) )
   return c_process_send_signal( proc, SIGCONT );
#else
   return BUNSPEC;
#endif
}
