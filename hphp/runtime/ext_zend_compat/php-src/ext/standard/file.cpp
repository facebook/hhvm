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
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Stig Bakken <ssb@php.net>                                   |
   |          Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   | PHP 4.0 patches by Thies C. Arntzen (thies@thieso.net)               |
   | PHP streams by Wez Furlong (wez@thebrainroom.com)                    |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* Synced with php 3.0 revision 1.218 1999-06-16 [ssb] */

/* {{{ includes */

#include "php.h"
#include "php_globals.h"
#include "php_smart_str.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef PHP_WIN32
# include <io.h>
# define O_RDONLY _O_RDONLY
# include "win32/param.h"
# include "win32/winutil.h"
# include "win32/fnmatch.h"
#else
# if HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif
# if HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
# if defined(NETWARE) && defined(USE_WINSOCK)
#  include <novsock2.h>
# else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
# endif
# if HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif

#include "ext/standard/head.h"
#include "php_string.h"
#include "file.h"

#if HAVE_PWD_H
# ifdef PHP_WIN32
#  include "win32/pwd.h"
# else
#  include <pwd.h>
# endif
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "fsock.h"
#include "fopen_wrappers.h"
#include "php_globals.h"

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#if MISSING_FCLOSE_DECL
extern int fclose(FILE *);
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#include "zend_API.h"

#ifdef ZTS
int file_globals_id;
#else
php_file_globals file_globals;
#endif

#if defined(HAVE_FNMATCH) && !defined(PHP_WIN32)
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <fnmatch.h>
#endif

#ifdef HAVE_WCHAR_H
# include <wchar.h>
#endif

#ifndef S_ISDIR
# define S_ISDIR(mode)  (((mode)&S_IFMT) == S_IFDIR)
#endif
/* }}} */

#define PHP_STREAM_TO_ZVAL(stream, arg) \
  php_stream_from_zval_no_verify(stream, arg); \
  if (stream == NULL) {  \
    RETURN_FALSE;  \
  }

/* {{{ ZTS-stuff / Globals / Prototypes */

/* sharing globals is *evil* */
static int le_stream_context = FAILURE;

PHPAPI int php_le_stream_context(TSRMLS_D)
{
  return le_stream_context;
}
/* }}} */
