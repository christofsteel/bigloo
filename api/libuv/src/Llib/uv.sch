;*=====================================================================*/
;*    serrano/prgm/project/bigloo/api/libuv/src/Llib/uv.sch            */
;*    -------------------------------------------------------------    */
;*    Author      :  Manuel Serrano                                    */
;*    Creation    :  Tue May  6 11:57:14 2014                          */
;*    Last change :  Mon Jul 28 14:13:29 2014 (serrano)                */
;*    Copyright   :  2014 Manuel Serrano                               */
;*    -------------------------------------------------------------    */
;*    LIBUV C bindings                                                 */
;*    -------------------------------------------------------------    */
;*    https://github.com/thlorenz/libuv-dox                            */
;*    http://nikhilm.github.io/uvbook/basics.html                      */
;*=====================================================================*/

;*---------------------------------------------------------------------*/
;*    The directives                                                   */
;*---------------------------------------------------------------------*/
(directives

   (extern
      (include "uv.h")

      ;; misc
      (macro $void*_nil::void* "0L")
      (macro $string-nil::string "0L")
      
      (type double* void* "double *")
      (macro $f64vector->double*::double* (::f64vector ::int) "&BGL_F64VREF")
      (macro $u8vector->double*::double* (::u8vector ::int) "&BGL_F64VREF")

      (macro $uv-strerror::string (::int) "(char *)uv_strerror")
      (macro $uv-err-name::string (::int) "(char *)uv_err_name")
      
      ;; handle
      (type $uv_handle_t void* "uv_handle_t *")
      (type $uv_close_cb void* "uv_close_cb")
      (macro $uv-handle-t::$uv_handle_t (::void*) "(uv_handle_t *)")
      
      (macro $uv_handle_nil::$uv_handle_t "0L")
      (macro $uv_handle_nilp::bool (::$uv_handle_t) "((uv_handle_t *)0L) == ")
      
      (macro $uv-handle-ref::void (::$uv_handle_t) "uv_ref")
      (macro $uv-handle-unref::void (::$uv_handle_t) "uv_unref")
      (macro $uv-handle-close::void (::$uv_handle_t ::$uv_close_cb) "uv_close")

      ($bgl_uv_close_cb::$uv_close_cb (::$uv_handle_t) "bgl_uv_close_cb")
      (macro $BGL_UV_CLOSE_CB::$uv_close_cb "(uv_close_cb)&bgl_uv_close_cb")
      
      ;; loop
      (type $uv_loop_t void* "uv_loop_t *")
      (macro $uv-loop-t::$uv_loop_t (::$uv_handle_t) "(uv_loop_t *)")
      
      (macro $uv_loop_nil::$uv_loop_t "0L")
      (macro $uv_loop_nilp::bool (::$uv_loop_t) "((uv_loop_t *)0L) == ")
      
      (macro $uv_loop_new::$uv_loop_t () "uv_loop_new")
      (macro $uv_default_loop::$uv_loop_t () "uv_default_loop")
      
      (macro $uv-run::void (::$uv_loop_t ::int) "uv_run")
      (macro $uv-stop::void (::$uv_loop_t) "uv_stop")
      
      (macro $UV_RUN_DEFAULT::int "UV_RUN_DEFAULT")

      ;; timer
      (type $uv_timer_t void* "uv_timer_t *")
      (type $uv_timer_cb void* "uv_timer_cb")
      (macro $uv-timer-t::$uv_timer_t (::$uv_handle_t) "(uv_timer_t *)")
      
      (macro $uv_timer_nil::$uv_timer_t "0L")
      
      ($bgl_uv_timer_new::$uv_timer_t (::UvTimer ::UvLoop) "bgl_uv_timer_new")
      (macro $uv_timer_start::void (::$uv_timer_t ::$uv_timer_cb ::uint64 ::uint64) "uv_timer_start")
      (macro $uv_timer_stop::void (::$uv_timer_t) "uv_timer_stop")

      ($bgl_uv_timer_cb::$uv_timer_cb (::$uv_timer_t ::int) "bgl_uv_handle_cb")
      (macro $BGL_UV_TIMER_CB::$uv_timer_cb "(uv_timer_cb)&bgl_uv_handle_cb")

      ;; idle
      (type $uv_idle_t void* "uv_idle_t *")
      (type $uv_idle_cb void* "uv_idle_cb")
      (macro $uv-idle-t::$uv_idle_t (::$uv_handle_t) "(uv_idle_t *)")
      
      (macro $uv_idle_nil::$uv_idle_t "0L")
      
      ($bgl_uv_idle_new::$uv_idle_t (::UvIdle ::UvLoop) "bgl_uv_idle_new")
      (macro $uv_idle_start::void (::$uv_idle_t ::$uv_idle_cb) "uv_idle_start")
      (macro $uv_idle_stop::void (::$uv_idle_t) "uv_idle_stop")

      ($bgl_uv_idle_cb::$uv_idle_cb (::$uv_idle_t ::int) "bgl_uv_handle_cb")
      (macro $BGL_UV_IDLE_CB::$uv_idle_cb "(uv_idle_cb)&bgl_uv_handle_cb")

      ;; async
      (type $uv_async_t void* "uv_async_t *")
      (type $uv_async_cb void* "uv_async_cb")
      (macro $uv-async-t::$uv_async_t (::$uv_handle_t) "(uv_async_t *)")
      
      (infix macro $uv_async_nil::$uv_async_t () "0L")
      
      ($bgl_uv_async_new::$uv_async_t (::UvAsync ::UvLoop) "bgl_uv_async_new")
      (macro $uv_async_send::void (::$uv_async_t) "uv_async_send")

      ;; fs
      ($uv-open-input-file::obj (::obj ::obj ::obj)
	 "bgl_uv_open_input_file")
      
      ($uv-fs-rename::int (::string ::string ::obj ::UvLoop)
	 "bgl_uv_fs_rename")
      ($uv-fs-ftruncate::int (::UvFile ::long ::obj ::UvLoop)
	 "bgl_uv_fs_ftruncate")
      ($uv-fs-truncate::int (::string ::long ::obj ::UvLoop)
	 "bgl_uv_fs_truncate")
      ($uv-fs-chown::int (::string ::int ::int ::obj ::UvLoop)
	 "bgl_uv_fs_chown")
      ($uv-fs-fchown::int (::UvFile ::int ::int ::obj ::UvLoop)
	 "bgl_uv_fs_fchown")
      ($uv-fs-lchown::int (::string ::int ::int ::obj ::UvLoop)
	 "bgl_uv_fs_lchown")
      ($uv-fs-chmod::int (::string ::int ::obj ::UvLoop)
	 "bgl_uv_fs_chmod")
      ($uv-fs-fchmod::int (::UvFile ::int ::obj ::UvLoop)
	 "bgl_uv_fs_fchmod")
      ($uv-fs-open::obj (::bstring ::int ::int ::obj ::UvLoop)
	 "bgl_uv_fs_open")
      ($uv-fs-close::int (::UvFile ::obj ::UvLoop)
	 "bgl_uv_fs_close")
      ($uv-fs-fstat::obj (::UvFile ::obj ::UvLoop)
	 "bgl_uv_fs_fstat")
      ($uv-fs-lstat::obj (::string ::obj ::UvLoop)
	 "bgl_uv_fs_lstat")
      ($uv-fs-stat::obj (::string ::obj ::UvLoop)
	 "bgl_uv_fs_stat")
      ($uv-fs-link::int (::string ::string ::obj ::UvLoop)
	 "bgl_uv_fs_link")
      ($uv-fs-symlink::int (::string ::string ::obj ::UvLoop)
	 "bgl_uv_fs_symlink")
      ($uv-fs-readlink::int (::string ::obj ::UvLoop)
	 "bgl_uv_fs_readlink")
      ($uv-fs-unlink::int (::string ::obj ::UvLoop)
	 "bgl_uv_fs_unlink")
      ($uv-fs-rmdir::int (::string ::obj ::UvLoop)
	 "bgl_uv_fs_rmdir")
      ($uv-fs-mkdir::int (::string ::int ::obj ::UvLoop)
	 "bgl_uv_fs_mkdir")
      ($uv-fs-fsync::int (::UvFile ::obj ::UvLoop)
	 "bgl_uv_fs_fsync")
      ($uv-fs-fdatasync::int (::UvFile ::obj ::UvLoop)
	 "bgl_uv_fs_fdatasync")
      ($uv-fs-futime::int (::UvFile ::double ::double ::obj ::UvLoop)
	 "bgl_uv_fs_futime")
      ($uv-fs-utime::int (::bstring ::double ::double ::obj ::UvLoop)
	 "bgl_uv_fs_utime")
      ($uv-fs-write::int (::UvFile ::bstring ::long ::long ::long ::obj ::UvLoop)
	 "bgl_uv_fs_write")
      ($uv-fs-read::int (::UvFile ::bstring ::long ::long ::long ::obj ::UvLoop)
	 "bgl_uv_fs_read")

      ;; os
      (macro $uv-loadavg::void (::double*) "uv_loadavg")
      (macro $uv-get-free-memory::double () "uv_get_free_memory")
      (macro $uv-get-total-memory::double () "uv_get_total_memory")
      ($uv-cpus::vector () "bgl_uv_cpus")

      ;; dns
      ($uv-getaddrinfo::int (::string ::string ::int ::obj ::UvLoop)
	 "bgl_uv_getaddrinfo")
      
      ))



