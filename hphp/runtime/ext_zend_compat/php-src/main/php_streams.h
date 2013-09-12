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
typedef struct _php_stream_wrapper php_stream_wrapper;
typedef struct _php_stream_context php_stream_context;
typedef struct _php_stream_filter php_stream_filter;

#include "streams/php_stream_transport.h"
#include "streams/php_stream_context.h"
#include "streams/php_stream_filter_api.h"

typedef struct _php_stream_statbuf {
	struct stat sb; /* regular info */
	/* extended info to go here some day: content-type etc. etc. */
} php_stream_statbuf;

/* operations on streams that are file-handles */
typedef struct _php_stream_ops  {
	/* stdio like functions - these are mandatory! */
	size_t (*write)(php_stream *stream, const char *buf, size_t count TSRMLS_DC);
	size_t (*read)(php_stream *stream, char *buf, size_t count TSRMLS_DC);
	int    (*close)(php_stream *stream, int close_handle TSRMLS_DC);
	int    (*flush)(php_stream *stream TSRMLS_DC);

	const char *label; /* label for this ops structure */

	/* these are optional */
	int (*seek)(php_stream *stream, off_t offset, int whence, off_t *newoffset TSRMLS_DC);
	int (*cast)(php_stream *stream, int castas, void **ret TSRMLS_DC);
	int (*stat)(php_stream *stream, php_stream_statbuf *ssb TSRMLS_DC);
	int (*set_option)(php_stream *stream, int option, int value, void *ptrparam TSRMLS_DC);
} php_stream_ops;

#define PHP_STREAM_FLAG_NO_SEEK						1
#define PHP_STREAM_FLAG_NO_BUFFER					2

#define PHP_STREAM_FLAG_EOL_UNIX					0 /* also includes DOS */
#define PHP_STREAM_FLAG_DETECT_EOL					4
#define PHP_STREAM_FLAG_EOL_MAC						8

/* set this when the stream might represent "interactive" data.
 * When set, the read buffer will avoid certain operations that
 * might otherwise cause the read to block for much longer than
 * is strictly required. */
#define PHP_STREAM_FLAG_AVOID_BLOCKING				16

#define PHP_STREAM_FLAG_NO_CLOSE					32

#define PHP_STREAM_FLAG_IS_DIR						64

#define PHP_STREAM_FLAG_NO_FCLOSE					128


struct _php_stream {
  _php_stream(HPHP::File *file) : hphp_file(file) {}
  HPHP::File *hphp_file;
	
  php_stream_ops *ops;
	void *abstract;			/* convenience pointer for abstraction */

	php_stream_filter_chain readfilters, writefilters;

	php_stream_wrapper *wrapper; /* which wrapper was used to open the stream */
	void *wrapperthis;		/* convenience pointer for a instance of a wrapper */
	zval *wrapperdata;		/* fgetwrapperdata retrieves this */

	int fgetss_state;		/* for fgetss to handle multiline tags */
	int is_persistent;
	char mode[16];			/* "rwb" etc. ala stdio */
	int rsrc_id;			/* used for auto-cleanup */
	int in_free;			/* to prevent recursion during free */
	/* so we know how to clean it up correctly.  This should be set to
	 * PHP_STREAM_FCLOSE_XXX as appropriate */
	int fclose_stdiocast;
	FILE *stdiocast;    /* cache this, otherwise we might leak! */
#if ZEND_DEBUG
	int __exposed;	/* non-zero if exposed as a zval somewhere */
#endif
	char *orig_path;

	php_stream_context *context;
	int flags;	/* PHP_STREAM_FLAG_XXX */

	/* buffer */
	off_t position; /* of underlying stream */
	unsigned char *readbuf;
	size_t readbuflen;
	off_t readpos;
	off_t writepos;

	/* how much data to read when filling buffer */
	size_t chunk_size;

	int eof;

#if ZEND_DEBUG
	const char *open_filename;
	uint open_lineno;
#endif

	struct _php_stream *enclosing_stream; /* this is a private stream owned by enclosing_stream */
}; /* php_stream */

#define php_stream_from_zval(xstr, ppzval) \
  { \
    HPHP::File *__file; \
    ZEND_FETCH_RESOURCE2((__file), HPHP::File *, (ppzval), -1, "stream", php_file_le_stream(), php_file_le_pstream()) \
    (xstr) = new (HPHP::request_arena()) php_stream(__file); \
  }

/* allocate a new stream for a particular ops */
BEGIN_EXTERN_C()
PHPAPI php_stream *_php_stream_alloc(php_stream_ops *ops, void *abstract,
		const char *persistent_id, const char *mode STREAMS_DC TSRMLS_DC);
END_EXTERN_C()
#define php_stream_alloc(ops, thisptr, persistent_id, mode)	_php_stream_alloc((ops), (thisptr), (persistent_id), (mode) STREAMS_CC TSRMLS_CC)

#define php_stream_get_resource_id(stream)		(stream)->rsrc_id
#if ZEND_DEBUG
/* use this to tell the stream that it is OK if we don't explicitly close it */
# define php_stream_auto_cleanup(stream)	{ (stream)->__exposed++; }
/* use this to assign the stream to a zval and tell the stream that is
 * has been exported to the engine; it will expect to be closed automatically
 * when the resources are auto-destructed */
# define php_stream_to_zval(stream, zval)	{ ZVAL_RESOURCE(zval, (stream)->rsrc_id); (stream)->__exposed++; }
#else
# define php_stream_auto_cleanup(stream)	/* nothing */
# define php_stream_to_zval(stream, zval)	{ ZVAL_RESOURCE(zval, (stream)->rsrc_id); }
#endif

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

PHPAPI int _php_stream_set_option(php_stream *stream, int option, int value, void *ptrparam TSRMLS_DC);
#define php_stream_set_option(stream, option, value, ptrvalue)	_php_stream_set_option((stream), (option), (value), (ptrvalue) TSRMLS_CC)

/* Flags for mkdir method in wrapper ops */
#define PHP_STREAM_MKDIR_RECURSIVE	1
/* define REPORT ERRORS 8 (below) */

/* Flags for rmdir method in wrapper ops */
/* define REPORT_ERRORS 8 (below) */

/* Flags for url_stat method in wrapper ops */
#define PHP_STREAM_URL_STAT_LINK	1
#define PHP_STREAM_URL_STAT_QUIET	2

/* change the blocking mode of stream: value == 1 => blocking, value == 0 => non-blocking. */
#define PHP_STREAM_OPTION_BLOCKING	1

/* change the buffering mode of stream. value is a PHP_STREAM_BUFFER_XXXX value, ptrparam is a ptr to a size_t holding
 * the required buffer size */
#define PHP_STREAM_OPTION_READ_BUFFER	2
#define PHP_STREAM_OPTION_WRITE_BUFFER	3

#define PHP_STREAM_BUFFER_NONE	0	/* unbuffered */
#define PHP_STREAM_BUFFER_LINE	1	/* line buffered */
#define PHP_STREAM_BUFFER_FULL	2	/* fully buffered */

/* set the timeout duration for reads on the stream. ptrparam is a pointer to a struct timeval * */
#define PHP_STREAM_OPTION_READ_TIMEOUT	4
#define PHP_STREAM_OPTION_SET_CHUNK_SIZE	5

/* set or release lock on a stream */
#define PHP_STREAM_OPTION_LOCKING		6

/* whether or not locking is supported */
#define PHP_STREAM_LOCK_SUPPORTED		1

#define php_stream_supports_lock(stream)	_php_stream_set_option((stream), PHP_STREAM_OPTION_LOCKING, 0, (void *) PHP_STREAM_LOCK_SUPPORTED TSRMLS_CC) == 0 ? 1 : 0
#define php_stream_lock(stream, mode)		_php_stream_set_option((stream), PHP_STREAM_OPTION_LOCKING, (mode), (void *) NULL TSRMLS_CC)

/* option code used by the php_stream_xport_XXX api */
#define PHP_STREAM_OPTION_XPORT_API			7 /* see php_stream_transport.h */
#define PHP_STREAM_OPTION_CRYPTO_API		8 /* see php_stream_transport.h */
#define PHP_STREAM_OPTION_MMAP_API			9 /* see php_stream_mmap.h */
#define PHP_STREAM_OPTION_TRUNCATE_API		10

#define PHP_STREAM_TRUNCATE_SUPPORTED	0
#define PHP_STREAM_TRUNCATE_SET_SIZE	1	/* ptrparam is a pointer to a size_t */

#define php_stream_truncate_supported(stream)	(_php_stream_set_option((stream), PHP_STREAM_OPTION_TRUNCATE_API, PHP_STREAM_TRUNCATE_SUPPORTED, NULL TSRMLS_CC) == PHP_STREAM_OPTION_RETURN_OK ? 1 : 0)

#define PHP_STREAM_OPTION_META_DATA_API		11 /* ptrparam is a zval* to which to add meta data information */
#define php_stream_populate_meta_data(stream, zv)	(_php_stream_set_option((stream), PHP_STREAM_OPTION_META_DATA_API, 0, zv TSRMLS_CC) == PHP_STREAM_OPTION_RETURN_OK ? 1 : 0)

/* Check if the stream is still "live"; for sockets/pipes this means the socket
 * is still connected; for files, this does not really have meaning */
#define PHP_STREAM_OPTION_CHECK_LIVENESS	12 /* no parameters */

#define PHP_STREAM_OPTION_RETURN_OK			 0 /* option set OK */
#define PHP_STREAM_OPTION_RETURN_ERR		-1 /* problem setting option */
#define PHP_STREAM_OPTION_RETURN_NOTIMPL	-2 /* underlying stream does not implement; streams can handle it instead */

/* copy up to maxlen bytes from src to dest.  If maxlen is PHP_STREAM_COPY_ALL,
 * copy until eof(src). */
#define PHP_STREAM_COPY_ALL		((size_t)-1)

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
