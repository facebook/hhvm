/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
*/
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"
#include "hphp/runtime/ext_zend_compat/php-src/TSRM/tsrm_virtual_cwd.h"
#include "hphp/runtime/ext_zend_compat/php-src/TSRM/TSRM.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/ext/ext_file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Unimplemented:
// CWD_API int virtual_file_ex(cwd_state *state, const char *path, verify_path_func verify_path, int use_realpath TSRMLS_DC)
// CWD_API int virtual_chdir_file(const char *path, int (*p_chdir)(const char *path TSRMLS_DC) TSRMLS_DC)
// CWD_API int virtual_filepath_ex(const char *path, char **filepath, verify_path_func verify_path TSRMLS_DC)
// CWD_API int virtual_filepath(const char *path, char **filepath TSRMLS_DC)
// CWD_API FILE *virtual_popen(const char *command, const char *type TSRMLS_DC)

CWD_API char *virtual_getcwd_ex(size_t *length TSRMLS_DC) /* {{{ */
{
	*length = HPHP::g_context->getCwd().size();
	return estrdup(HPHP::g_context->getCwd().c_str());
}
/* }}} */

CWD_API char *virtual_getcwd(char *buf, size_t size TSRMLS_DC) /* {{{ */
{
	int cwd_size = HPHP::g_context->getCwd().size();
	if (cwd_size >= size) {
		errno = ERANGE;
		return NULL;
	} else {
		memcpy(buf, HPHP::g_context->getCwd().data(), cwd_size);
		buf[cwd_size] = '\0';
		return buf;
	}
}
/* }}} */

CWD_API int virtual_chdir(const char *path TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	struct stat s;
	if (stat(translated.c_str(), &s) != 0) {
		return -1; // failure
	}
	if (!S_ISDIR(s.st_mode)) {
		errno = ENOTDIR;
		return -1; // failure
	}
	HPHP::g_context->setCwd(translated);
	return 0;
}
/* }}} */
CWD_API char *virtual_realpath(const char *path, char *real_path TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	if (translated.empty()) {
		return NULL;
	}
	if (!realpath(translated.c_str(), real_path)) {
		return NULL;
	}
	return real_path;
}
/* }}} */

CWD_API FILE *virtual_fopen(const char *path, const char *mode TSRMLS_DC) /* {{{ */
{
	if (path[0] == '\0') { /* Fail to open empty path */
		return NULL;
	}
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	return fopen(translated.c_str(), mode);
}
/* }}} */

CWD_API int virtual_access(const char *pathname, int mode TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(pathname, HPHP::CopyString));
	return access(translated.c_str(), mode);	
}
/* }}} */

CWD_API int virtual_utime(const char *filename, struct utimbuf *buf TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(filename, HPHP::CopyString));
	return utime(translated.c_str(), buf);	
}
/* }}} */

CWD_API int virtual_chmod(const char *filename, mode_t mode TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(filename, HPHP::CopyString));
	return chmod(translated.c_str(), mode);
}
/* }}} */

CWD_API int virtual_chown(const char *filename, uid_t owner, gid_t group, int link TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(filename, HPHP::CopyString));
	int ret;
	if (link) {
#if HAVE_LCHOWN
		ret = lchown(translated.c_str(), owner, group);
#else
		ret = -1;
#endif
	} else {
		ret = chown(translated.c_str(), owner, group);
	}
	return ret;
}
/* }}} */


CWD_API int virtual_open(const char *path TSRMLS_DC, int flags, ...) /* {{{ */
{
	int f;
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	if (flags & O_CREAT) {
		mode_t mode;
		va_list arg;

		va_start(arg, flags);
		mode = (mode_t) va_arg(arg, int);
		va_end(arg);

		f = open(translated.c_str(), flags, mode);
	} else {
		f = open(translated.c_str(), flags);
	}
	return f;
}
/* }}} */

CWD_API int virtual_creat(const char *path, mode_t mode TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	return creat(translated.c_str(), mode);
}
/* }}} */

CWD_API int virtual_rename(const char *oldname, const char *newname TSRMLS_DC) /* {{{ */
{
	HPHP::String oldTrans = HPHP::File::TranslatePath(HPHP::String(oldname, HPHP::CopyString));
	HPHP::String newTrans = HPHP::File::TranslatePath(HPHP::String(newname, HPHP::CopyString));
	return rename(oldTrans.c_str(), newTrans.c_str());
}
/* }}} */

CWD_API int virtual_stat(const char *path, struct stat *buf TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	return stat(translated.c_str(), buf);
}
/* }}} */

CWD_API int virtual_lstat(const char *path, struct stat *buf TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	return lstat(translated.c_str(), buf);
}
/* }}} */

CWD_API int virtual_unlink(const char *path TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(path, HPHP::CopyString));
	return unlink(translated.c_str());
}
/* }}} */

CWD_API int virtual_mkdir(const char *pathname, mode_t mode TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(pathname, HPHP::CopyString));
	return mkdir(translated.c_str(), mode);
}
/* }}} */
CWD_API int virtual_rmdir(const char *pathname TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(pathname, HPHP::CopyString));
	return rmdir(translated.c_str());
}
/* }}} */

CWD_API DIR *virtual_opendir(const char *pathname TSRMLS_DC) /* {{{ */
{
	HPHP::String translated = HPHP::File::TranslatePath(HPHP::String(pathname, HPHP::CopyString));
	return opendir(translated.c_str());
}
/* }}} */
