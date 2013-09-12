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
  | Author: Wez Furlong <wez@thebrainroom.com>                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
#ifdef PHP_WIN32
#include "config.w32.h"
#include <Ws2tcpip.h>
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

BEGIN_EXTERN_C()

#define STREAM_XPORT_CLIENT			0
#define STREAM_XPORT_SERVER			1

#define STREAM_XPORT_CONNECT		2
#define STREAM_XPORT_BIND			4
#define STREAM_XPORT_LISTEN			8
#define STREAM_XPORT_CONNECT_ASYNC	16

/* Open a client or server socket connection */
PHPAPI php_stream *_php_stream_xport_create(const char *name, long namelen, int options,
		int flags, const char *persistent_id,
		struct timeval *timeout,
		php_stream_context *context,
		char **error_string,
		int *error_code
		STREAMS_DC TSRMLS_DC);

#define php_stream_xport_create(name, namelen, options, flags, persistent_id, timeout, context, estr, ecode) \
	_php_stream_xport_create(name, namelen, options, flags, persistent_id, timeout, context, estr, ecode STREAMS_CC TSRMLS_CC)

/* These functions provide crypto support on the underlying transport */
typedef enum {
	STREAM_CRYPTO_METHOD_SSLv2_CLIENT,
	STREAM_CRYPTO_METHOD_SSLv3_CLIENT,
	STREAM_CRYPTO_METHOD_SSLv23_CLIENT,
	STREAM_CRYPTO_METHOD_TLS_CLIENT,
	STREAM_CRYPTO_METHOD_SSLv2_SERVER,
	STREAM_CRYPTO_METHOD_SSLv3_SERVER,
	STREAM_CRYPTO_METHOD_SSLv23_SERVER,
	STREAM_CRYPTO_METHOD_TLS_SERVER
} php_stream_xport_crypt_method_t;

BEGIN_EXTERN_C()
PHPAPI int php_stream_xport_crypto_setup(php_stream *stream, php_stream_xport_crypt_method_t crypto_method, php_stream *session_stream TSRMLS_DC);
PHPAPI int php_stream_xport_crypto_enable(php_stream *stream, int activate TSRMLS_DC);
END_EXTERN_C()

END_EXTERN_C()
