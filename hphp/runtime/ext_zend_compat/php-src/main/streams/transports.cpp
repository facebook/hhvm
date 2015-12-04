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

#include "php.h"
#include "hphp/runtime/base/exceptions.h"

PHPAPI HashTable *php_stream_xport_get_hash(void)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_get_hash");
}

PHPAPI int php_stream_xport_register(char *protocol, php_stream_transport_factory factory TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_register");
}

PHPAPI int php_stream_xport_unregister(char *protocol TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_unregister");
}

PHPAPI php_stream *_php_stream_xport_create(const char *name, long namelen, int options,
    int flags, const char *persistent_id,
    struct timeval *timeout,
    php_stream_context *context,
    char **error_string,
    int *error_code
    STREAMS_DC TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: _php_stream_xport_create");
}

/* Bind the stream to a local address */
PHPAPI int php_stream_xport_bind(php_stream *stream,
    const char *name, long namelen,
    char **error_text
    TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_bind");
}

/* Connect to a remote address */
PHPAPI int php_stream_xport_connect(php_stream *stream,
    const char *name, long namelen,
    int asynchronous,
    struct timeval *timeout,
    char **error_text,
    int *error_code
    TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_connect");
}

/* Prepare to listen */
PHPAPI int php_stream_xport_listen(php_stream *stream, int backlog, char **error_text TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_listen");
}

/* Get the next client and their address (as a string) */
PHPAPI int php_stream_xport_accept(php_stream *stream, php_stream **client,
    char **textaddr, int *textaddrlen,
    void **addr, socklen_t *addrlen,
    struct timeval *timeout,
    char **error_text
    TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_accept");
}

PHPAPI int php_stream_xport_get_name(php_stream *stream, int want_peer,
    char **textaddr, int *textaddrlen,
    void **addr, socklen_t *addrlen
    TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_get_name");
}

PHPAPI int php_stream_xport_crypto_setup(php_stream *stream, php_stream_xport_crypt_method_t crypto_method, php_stream *session_stream TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_crypto_setup");
}

PHPAPI int php_stream_xport_crypto_enable(php_stream *stream, int activate TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_crypto_enable");
}

/* Similar to recv() system call; read data from the stream, optionally
 * peeking, optionally retrieving OOB data */
PHPAPI int php_stream_xport_recvfrom(php_stream *stream, char *buf, size_t buflen,
    long flags, void **addr, socklen_t *addrlen, char **textaddr, int *textaddrlen
    TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_recvfrom");
}

/* Similar to send() system call; send data to the stream, optionally
 * sending it as OOB data */
PHPAPI int php_stream_xport_sendto(php_stream *stream, const char *buf, size_t buflen,
    long flags, void *addr, socklen_t addrlen TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_sendto");
}

/* Similar to shutdown() system call; shut down part of a full-duplex
 * connection */
PHPAPI int php_stream_xport_shutdown(php_stream *stream, stream_shutdown_t how TSRMLS_DC)
{
  HPHP::raise_fatal_error("unimplemented: php_stream_xport_shutdown");
}
