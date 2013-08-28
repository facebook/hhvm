/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong (wez@thebrainroom.com)                           |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifndef PHP_STREAMS_H
#define PHP_STREAMS_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "php.h"
#include "hphp/runtime/base/php-stream-wrapper.h"

BEGIN_EXTERN_C()
PHPAPI int php_file_le_stream(void);
PHPAPI int php_file_le_pstream(void);
PHPAPI int php_file_le_stream_filter(void);
END_EXTERN_C()

#define STREAMS_CC
#define STREAMS_DC

#define php_stream_fopen_tmpfile_rel()  _php_stream_fopen_tmpfile(0 STREAMS_REL_CC TSRMLS_CC)

/* The contents of the php_stream_ops and php_stream should only be accessed
 * using the functions/macros in this header.
 * If you need to get at something that doesn't have an API,
 * drop me a line <wez@thebrainroom.com> and we can sort out a way to do
 * it properly.
 *
 * The only exceptions to this rule are that stream implementations can use
 * the php_stream->abstract pointer to hold their context, and streams
 * opened via stream_open_wrappers can use the zval ptr in
 * php_stream->wrapperdata to hold meta data for php scripts to
 * retrieve using file_get_wrapper_data(). */

typedef struct _php_stream php_stream;
typedef struct _php_stream_context php_stream_context;

#include "streams/php_stream_context.h"

struct _php_stream {
  _php_stream(HPHP::File *file) : hphp_file(file) {}
  HPHP::File *hphp_file;

  void *abstract;         /* convenience pointer for abstraction */
  zval *wrapperdata;      /* fgetwrapperdata retrieves this */
};

#define php_stream_from_zval(xstr, ppzval) \
  { \
    HPHP::File *__file; \
    ZEND_FETCH_RESOURCE2((__file), HPHP::File *, (ppzval), -1, "stream", php_file_le_stream(), php_file_le_pstream()) \
    (xstr) = new (HPHP::request_arena()) php_stream(__file); \
  }

#define PHP_STREAM_FREE_CALL_DTOR      1 /* call ops->close */
#define PHP_STREAM_FREE_RELEASE_STREAM    2 /* pefree(stream) */
#define PHP_STREAM_FREE_PRESERVE_HANDLE    4 /* tell ops->close to not close it's underlying handle */
#define PHP_STREAM_FREE_RSRC_DTOR      8 /* called from the resource list dtor */
#define PHP_STREAM_FREE_PERSISTENT      16 /* manually freeing a persistent connection */
#define PHP_STREAM_FREE_IGNORE_ENCLOSING  32 /* don't close the enclosing stream instead */
#define PHP_STREAM_FREE_CLOSE        (PHP_STREAM_FREE_CALL_DTOR | PHP_STREAM_FREE_RELEASE_STREAM)
#define PHP_STREAM_FREE_CLOSE_CASTED    (PHP_STREAM_FREE_CLOSE | PHP_STREAM_FREE_PRESERVE_HANDLE)
#define PHP_STREAM_FREE_CLOSE_PERSISTENT  (PHP_STREAM_FREE_CLOSE | PHP_STREAM_FREE_PERSISTENT)

BEGIN_EXTERN_C()
PHPAPI inline int _php_stream_free(php_stream *stream, int close_options TSRMLS_DC) {
  decRefRes(stream->hphp_file);
  return 1;
}
#define php_stream_free(stream, close_options)  _php_stream_free((stream), (close_options) TSRMLS_CC)
#define php_stream_close(stream)  _php_stream_free((stream), PHP_STREAM_FREE_CLOSE TSRMLS_CC)
#define php_stream_pclose(stream)  _php_stream_free((stream), PHP_STREAM_FREE_CLOSE_PERSISTENT TSRMLS_CC)

PHPAPI inline int _php_stream_seek(php_stream *stream, off_t offset, int whence TSRMLS_DC) {
  return stream->hphp_file->seek(offset, whence);
}
#define php_stream_rewind(stream)  _php_stream_seek((stream), 0L, SEEK_SET TSRMLS_CC)
#define php_stream_seek(stream, offset, whence)  _php_stream_seek((stream), (offset), (whence) TSRMLS_CC)

PHPAPI inline off_t _php_stream_tell(php_stream *stream TSRMLS_DC) {
  return stream->hphp_file->tell();
}
#define php_stream_tell(stream)  _php_stream_tell((stream) TSRMLS_CC)

PHPAPI inline size_t _php_stream_read(php_stream *stream, char *buf, size_t count TSRMLS_DC) {
  return stream->hphp_file->readImpl(buf, count);
}
#define php_stream_read(stream, buf, count)    _php_stream_read((stream), (buf), (count) TSRMLS_CC)

PHPAPI inline size_t _php_stream_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC) {
  return stream->hphp_file->writeImpl(buf, count);
}
#define php_stream_write_string(stream, str)  _php_stream_write(stream, str, strlen(str) TSRMLS_CC)
#define php_stream_write(stream, buf, count)  _php_stream_write(stream, (buf), (count) TSRMLS_CC)

PHPAPI inline int _php_stream_eof(php_stream *stream TSRMLS_DC) {
  return stream->hphp_file->eof();
}
#define php_stream_eof(stream)  _php_stream_eof((stream) TSRMLS_CC)

PHPAPI inline int _php_stream_getc(php_stream *stream TSRMLS_DC) {
  return stream->hphp_file->getc();
}
#define php_stream_getc(stream)  _php_stream_getc((stream) TSRMLS_CC)

PHPAPI inline int _php_stream_putc(php_stream *stream, int c TSRMLS_DC) {
  return stream->hphp_file->putc(c);
}
#define php_stream_putc(stream, c)  _php_stream_putc((stream), (c) TSRMLS_CC)


/* copy up to maxlen bytes from src to dest.  If maxlen is PHP_STREAM_COPY_ALL,
 * copy until eof(src). */
#define PHP_STREAM_COPY_ALL    ((size_t)-1)

#include "streams/php_stream_plain_wrapper.h"

/* read all data from stream and put into a buffer. Caller must free buffer
 * when done. */
PHPAPI size_t _php_stream_copy_to_mem(php_stream *src, char **buf, size_t maxlen, int persistent STREAMS_DC TSRMLS_DC);
#define php_stream_copy_to_mem(src, buf, maxlen, persistent) _php_stream_copy_to_mem((src), (buf), (maxlen), (persistent) STREAMS_CC TSRMLS_CC)

END_EXTERN_C()

/* coerce the stream into some other form */
/* cast as a stdio FILE * */
#define PHP_STREAM_AS_STDIO     0
/* cast as a POSIX fd or socketd */
#define PHP_STREAM_AS_FD    1
/* cast as a socketd */
#define PHP_STREAM_AS_SOCKETD  2
/* cast as fd/socket for select purposes */
#define PHP_STREAM_AS_FD_FOR_SELECT 3

/* try really, really hard to make sure the cast happens (avoid using this flag if possible) */
#define PHP_STREAM_CAST_TRY_HARD  0x80000000
#define PHP_STREAM_CAST_RELEASE    0x40000000  /* stream becomes invalid on success */
#define PHP_STREAM_CAST_INTERNAL  0x20000000  /* stream cast for internal use */
#define PHP_STREAM_CAST_MASK    (PHP_STREAM_CAST_TRY_HARD | PHP_STREAM_CAST_RELEASE | PHP_STREAM_CAST_INTERNAL)
BEGIN_EXTERN_C()
PHPAPI int _php_stream_cast(php_stream *stream, int castas, void **ret, int show_err TSRMLS_DC);
END_EXTERN_C()
/* use this to check if a stream can be cast into another form */
#define php_stream_can_cast(stream, as)  _php_stream_cast((stream), (as), NULL, 0 TSRMLS_CC)
#define php_stream_cast(stream, as, ret, show_err)  _php_stream_cast((stream), (as), (ret), (show_err) TSRMLS_CC)

/* Wrappers support */

#define IGNORE_PATH                     0x00000000
#define USE_PATH                        0x00000001
#define IGNORE_URL                      0x00000002
#define REPORT_ERRORS                   0x00000008
#define ENFORCE_SAFE_MODE               0 /* for BC only */

/* If you don't need to write to the stream, but really need to
 * be able to seek, use this flag in your options. */
#define STREAM_MUST_SEEK                0x00000010
/* If you are going to end up casting the stream into a FILE* or
 * a socket, pass this flag and the streams/wrappers will not use
 * buffering mechanisms while reading the headers, so that HTTP
 * wrapped streams will work consistently.
 * If you omit this flag, streams will use buffering and should end
 * up working more optimally.
 * */
#define STREAM_WILL_CAST                0x00000020

/* this flag applies to php_stream_locate_url_wrapper */
#define STREAM_LOCATE_WRAPPERS_ONLY     0x00000040

/* this flag is only used by include/require functions */
#define STREAM_OPEN_FOR_INCLUDE         0x00000080

/* this flag tells streams to ONLY open urls */
#define STREAM_USE_URL                  0x00000100

/* this flag is used when only the headers from HTTP request are to be fetched */
#define STREAM_ONLY_GET_HEADERS         0x00000200

/* don't apply open_basedir checks */
#define STREAM_DISABLE_OPEN_BASEDIR     0x00000400

/* get (or create) a persistent version of the stream */
#define STREAM_OPEN_PERSISTENT          0x00000800

/* use glob stream for directory open in plain files stream */
#define STREAM_USE_GLOB_DIR_OPEN        0x00001000

/* don't check allow_url_fopen and allow_url_include */
#define STREAM_DISABLE_URL_PROTECTION   0x00002000

/* assume the path passed in exists and is fully expanded, avoiding syscalls */
#define STREAM_ASSUME_REALPATH          0x00004000

BEGIN_EXTERN_C()
PHPAPI php_stream *_php_stream_open_wrapper_ex(char *path, const char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC);

#define php_stream_open_wrapper(path, mode, options, opened)  _php_stream_open_wrapper_ex((path), (mode), (options), (opened), NULL STREAMS_CC TSRMLS_CC)
#define php_stream_open_wrapper_ex(path, mode, options, opened, context)  _php_stream_open_wrapper_ex((path), (mode), (options), (opened), (context) STREAMS_CC TSRMLS_CC)
END_EXTERN_C()
#endif
