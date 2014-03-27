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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Jim Winstead <jimw@php.net>                                 |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/* {{{ includes
 */
#include "php.h"
#include "php_globals.h"
#include "SAPI.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef PHP_WIN32
#define O_RDONLY _O_RDONLY
#include "win32/param.h"
#else
#include <sys/param.h>
#endif

#include "ext/standard/head.h"
#include "zend_compile.h"
#include "php_network.h"

#if HAVE_PWD_H
#include <pwd.h>
#endif

#include <sys/types.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef S_ISREG
#define S_ISREG(mode)	(((mode) & S_IFMT) == S_IFREG)
#endif

#ifdef PHP_WIN32
#include <winsock2.h>
#elif defined(NETWARE) && defined(USE_WINSOCK)
#include <novsock2.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif

#if defined(PHP_WIN32) || defined(__riscos__) || defined(NETWARE)
#undef AF_UNIX
#endif

#if defined(AF_UNIX)
#include <sys/un.h>
#endif
/* }}} */

#include "hphp/util/file-util.cpp"

PHPAPI int php_check_open_basedir(const char *path TSRMLS_DC) {
  // we don't support openbasedir so you can access anything
  return SUCCESS;
}

/* {{{ expand_filepath
 *  */
PHPAPI char *expand_filepath(const char *filepath, char *real_path TSRMLS_DC)
{
      return expand_filepath_ex(filepath, real_path, NULL, 0 TSRMLS_CC);
}
/* }}} */

/* {{{ expand_filepath_ex
 *  */
PHPAPI char *expand_filepath_ex(const char *filepath, char *real_path, const char *relative_to, size_t relative_to_len TSRMLS_DC)
{
      return expand_filepath_with_mode(filepath, real_path, relative_to, relative_to_len, CWD_FILEPATH TSRMLS_CC);
}
/* }}} */

PHPAPI char *expand_filepath_with_mode(const char *filepath, char *real_path, const char *relative_to, size_t relative_to_len, int realpath_mode TSRMLS_DC)
{
	// This part is basically the same as File::TranslatePath(), except relative_to is
	// optionally a parameter instead of coming from g_context
	assert(realpath_mode != CWD_FILEPATH); // not implemented
	if (realpath_mode == CWD_FILEPATH) {
		realpath_mode = CWD_REALPATH;
	}
	if (!filepath[0]) {
		return NULL;
	}

	HPHP::String canonicalized(HPHP::FileUtil::canonicalize(filepath, strlen(filepath)), HPHP::AttachString);
    if (canonicalized.charAt(0) != '/') {
		if (relative_to) {
			canonicalized = HPHP::String(relative_to, HPHP::CopyString) + "/" + canonicalized;
		} else {
			canonicalized = HPHP::g_context->getCwd() + "/" + canonicalized;
		}
	}
	
	if (real_path) {
		int copy_len;
		copy_len = canonicalized.size() > MAXPATHLEN - 1 ? MAXPATHLEN - 1 : canonicalized.size();
		memcpy(real_path, canonicalized.data(), copy_len);
		real_path[copy_len] = '\0';
	} else {
		real_path = estrndup(canonicalized.data(), canonicalized.size());
	}

	return real_path;
}
