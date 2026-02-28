/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/
#pragma once

/* $Id: acconfig.h,v 1.40.2.1.2.1 2007/01/01 09:35:45 sebastian Exp $ */
#define ZEND_API
#define ZEND_DLEXPORT
#define ZEND_DLIMPORT


/* #undef uint */
/* #undef ulong */

/* Define if you want to enable memory limit support */
#define MEMORY_LIMIT 0


/* */
/* #undef AIX */

/* */
/* #undef APC_FUTEX_LOCKS */

/* */
#define APC_MMAP 1

/* */
#define APC_PTHREADMUTEX_LOCKS 1

/* */
#define APC_REQFILES 1

/* */
/* #undef APC_SEM_LOCKS */

/* */
/* #undef APC_SPIN_LOCKS */

/* Whether to use native BeOS threads */
/* #undef BETHREADS */

/* Enabling BIND8 compatibility for Panther */
/* #undef BIND_8_COMPAT */

/* */
/* #undef CDB_INCLUDE_FILE */

/* Define if system uses EBCDIC */
/* #undef CHARSET_EBCDIC */

/* Whether to build apc as dynamic module */
/* #undef COMPILE_DL_APC */

/* Whether to build bcmath as dynamic module */
/* #undef COMPILE_DL_BCMATH */

/* Whether to build bz2 as dynamic module */
/* #undef COMPILE_DL_BZ2 */

/* Whether to build calendar as dynamic module */
/* #undef COMPILE_DL_CALENDAR */

/* Whether to build ctype as dynamic module */
/* #undef COMPILE_DL_CTYPE */

/* Whether to build curl as dynamic module */
/* #undef COMPILE_DL_CURL */

/* Whether to build date as dynamic module */
/* #undef COMPILE_DL_DATE */

/* Whether to build dba as dynamic module */
/* #undef COMPILE_DL_DBA */

/* Whether to build dbase as dynamic module */
/* #undef COMPILE_DL_DBASE */

/* Whether to build dom as dynamic module */
/* #undef COMPILE_DL_DOM */

/* Whether to build exif as dynamic module */
/* #undef COMPILE_DL_EXIF */

/* Whether to build fbsql as dynamic module */
/* #undef COMPILE_DL_FBSQL */

/* Whether to build fdf as dynamic module */
/* #undef COMPILE_DL_FDF */

/* Whether to build filter as dynamic module */
/* #undef COMPILE_DL_FILTER */

/* Whether to build ftp as dynamic module */
/* #undef COMPILE_DL_FTP */

/* Whether to build gd as dynamic module */
/* #undef COMPILE_DL_GD */

/* Whether to build gettext as dynamic module */
/* #undef COMPILE_DL_GETTEXT */

/* Whether to build gmp as dynamic module */
/* #undef COMPILE_DL_GMP */

/* Whether to build hash as dynamic module */
/* #undef COMPILE_DL_HASH */

/* Whether to build iconv as dynamic module */
/* #undef COMPILE_DL_ICONV */

/* Whether to build imap as dynamic module */
/* #undef COMPILE_DL_IMAP */

/* Whether to build interbase as dynamic module */
/* #undef COMPILE_DL_INTERBASE */

/* Whether to build json as dynamic module */
/* #undef COMPILE_DL_JSON */

/* Whether to build ldap as dynamic module */
/* #undef COMPILE_DL_LDAP */

/* Whether to build libxml as dynamic module */
/* #undef COMPILE_DL_LIBXML */

/* Whether to build mbstring as dynamic module */
/* #undef COMPILE_DL_MBSTRING */

/* Whether to build mcrypt as dynamic module */
/* #undef COMPILE_DL_MCRYPT */

/* Whether to build mhash as dynamic module */
/* #undef COMPILE_DL_MHASH */

/* Whether to build mime_magic as dynamic module */
/* #undef COMPILE_DL_MIME_MAGIC */

/* Whether to build ming as dynamic module */
/* #undef COMPILE_DL_MING */

/* Whether to build msql as dynamic module */
/* #undef COMPILE_DL_MSQL */

/* Whether to build mssql as dynamic module */
/* #undef COMPILE_DL_MSSQL */

/* Whether to build mysql as dynamic module */
/* #undef COMPILE_DL_MYSQL */

/* Whether to build mysqli as dynamic module */
/* #undef COMPILE_DL_MYSQLI */

/* Whether to build ncurses as dynamic module */
/* #undef COMPILE_DL_NCURSES */

/* Whether to build oci8 as dynamic module */
/* #undef COMPILE_DL_OCI8 */

/* Whether to build odbc as dynamic module */
/* #undef COMPILE_DL_ODBC */

/* Whether to build openssl as dynamic module */
/* #undef COMPILE_DL_OPENSSL */

/* Whether to build pcntl as dynamic module */
/* #undef COMPILE_DL_PCNTL */

/* Whether to build pcre as dynamic module */
/* #undef COMPILE_DL_PCRE */

/* Whether to build pdo as dynamic module */
/* #undef COMPILE_DL_PDO */

/* Whether to build pdo_dblib as dynamic module */
/* #undef COMPILE_DL_PDO_DBLIB */

/* Whether to build pdo_firebird as dynamic module */
/* #undef COMPILE_DL_PDO_FIREBIRD */

/* Whether to build pdo_mysql as dynamic module */
/* #undef COMPILE_DL_PDO_MYSQL */

/* Whether to build pdo_oci as dynamic module */
/* #undef COMPILE_DL_PDO_OCI */

/* Whether to build pdo_odbc as dynamic module */
/* #undef COMPILE_DL_PDO_ODBC */

/* Whether to build pdo_sqlite as dynamic module */
/* #undef COMPILE_DL_PDO_SQLITE */

/* Whether to build posix as dynamic module */
/* #undef COMPILE_DL_POSIX */

/* Whether to build pspell as dynamic module */
/* #undef COMPILE_DL_PSPELL */

/* Whether to build readline as dynamic module */
/* #undef COMPILE_DL_READLINE */

/* Whether to build recode as dynamic module */
/* #undef COMPILE_DL_RECODE */

/* Whether to build reflection as dynamic module */
/* #undef COMPILE_DL_REFLECTION */

/* Whether to build session as dynamic module */
/* #undef COMPILE_DL_SESSION */

/* Whether to build shmop as dynamic module */
/* #undef COMPILE_DL_SHMOP */

/* Whether to build simplexml as dynamic module */
/* #undef COMPILE_DL_SIMPLEXML */

/* Whether to build snmp as dynamic module */
/* #undef COMPILE_DL_SNMP */

/* Whether to build soap as dynamic module */
/* #undef COMPILE_DL_SOAP */

/* Whether to build sockets as dynamic module */
/* #undef COMPILE_DL_SOCKETS */

/* Whether to build spl as dynamic module */
/* #undef COMPILE_DL_SPL */

/* Whether to build sqlite as dynamic module */
/* #undef COMPILE_DL_SQLITE */

/* Whether to build standard as dynamic module */
/* #undef COMPILE_DL_STANDARD */

/* Whether to build sybase as dynamic module */
/* #undef COMPILE_DL_SYBASE */

/* Whether to build sybase_ct as dynamic module */
/* #undef COMPILE_DL_SYBASE_CT */

/* Whether to build sysvmsg as dynamic module */
/* #undef COMPILE_DL_SYSVMSG */

/* Whether to build sysvsem as dynamic module */
/* #undef COMPILE_DL_SYSVSEM */

/* Whether to build sysvshm as dynamic module */
/* #undef COMPILE_DL_SYSVSHM */

/* Whether to build tidy as dynamic module */
/* #undef COMPILE_DL_TIDY */

/* Whether to build tokenizer as dynamic module */
/* #undef COMPILE_DL_TOKENIZER */

/* Whether to build xml as dynamic module */
/* #undef COMPILE_DL_XML */

/* Whether to build xmlreader as dynamic module */
/* #undef COMPILE_DL_XMLREADER */

/* Whether to build xmlrpc as dynamic module */
/* #undef COMPILE_DL_XMLRPC */

/* Whether to build xmlwriter as dynamic module */
/* #undef COMPILE_DL_XMLWRITER */

/* Whether to build xsl as dynamic module */
/* #undef COMPILE_DL_XSL */

/* Whether to build zip as dynamic module */
/* #undef COMPILE_DL_ZIP */

/* Whether to build zlib as dynamic module */
/* #undef COMPILE_DL_ZLIB */

/* */
#define COOKIE_IO_FUNCTIONS_T cookie_io_functions_t

/* */
#define COOKIE_SEEKER_USES_OFF64_T 1

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define if crypt_r has uses CRYPTD */
/* #undef CRYPT_R_CRYPTD */

/* Define if struct crypt_data requires _GNU_SOURCE */
/* #undef CRYPT_R_GNU_SOURCE */

/* Define if crypt_r uses struct crypt_data */
/* #undef CRYPT_R_STRUCT_CRYPT_DATA */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define if the target system is darwin */
/* #undef DARWIN */

/* */
/* #undef DB1_INCLUDE_FILE */

/* */
/* #undef DB1_VERSION */

/* */
/* #undef DB2_INCLUDE_FILE */

/* */
/* #undef DB3_INCLUDE_FILE */

/* */
/* #undef DB4_INCLUDE_FILE */

/* */
/* #undef DBASE */

/* */
/* #undef DBA_CDB */

/* */
/* #undef DBA_CDB_BUILTIN */

/* */
/* #undef DBA_CDB_MAKE */

/* */
/* #undef DBA_DB1 */

/* */
/* #undef DBA_DB2 */

/* */
/* #undef DBA_DB3 */

/* */
/* #undef DBA_DB4 */

/* */
/* #undef DBA_DBM */

/* */
/* #undef DBA_FLATFILE */

/* */
/* #undef DBA_GDBM */

/* */
/* #undef DBA_INIFILE */

/* */
/* #undef DBA_NDBM */

/* */
/* #undef DBA_QDBM */

/* */
/* #undef DBMFIX */

/* */
/* #undef DBM_INCLUDE_FILE */

/* */
/* #undef DBM_VERSION */

/* */
#define DEFAULT_SHORT_OPEN_TAG "1"

/* */
/* #undef DISCARD_PATH */

/* Define if dlsym() requires a leading underscore in symbol names. */
/* #undef DLSYM_NEEDS_UNDERSCORE */

/* Whether to enable chroot() function */
/* #undef ENABLE_CHROOT_FUNC */

/* */
/* #undef ENABLE_PATHINFO_CHECK */

/* */
/* #undef FORCE_CGI_REDIRECT */

/* */
/* #undef GDBM_INCLUDE_FILE */

/* Whether you use GNU Pth */
/* #undef GNUPTH */

/* Whether 3 arg set_rebind_proc() */
/* #undef HAVE_3ARG_SETREBINDPROC */

/* Define to 1 if you have the `acosh' function. */
#define HAVE_ACOSH 1

/* */
/* #undef HAVE_ADABAS */

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#define HAVE_ALLOCA_H 1

/* Define to 1 if you have the `alphasort' function. */
#define HAVE_ALPHASORT 1

/* Whether you have AOLserver */
/* #undef HAVE_AOLSERVER */

/* */
#define HAVE_APACHE 1

/* */
/* #undef HAVE_APACHE_HOOKS */

/* */
#define HAVE_APC 1

/* Define to 1 if you have the <ApplicationServices/ApplicationServices.h>
   header file. */
/* #undef HAVE_APPLICATIONSERVICES_APPLICATIONSERVICES_H */

/* */
#define HAVE_AP_COMPAT_H 1

/* */
#define HAVE_AP_CONFIG_H 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <arpa/nameser.h> header file. */
#define HAVE_ARPA_NAMESER_H 1

/* Define to 1 if you have the `asctime_r' function. */
#define HAVE_ASCTIME_R 1

/* Define to 1 if you have the `asinh' function. */
#define HAVE_ASINH 1

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define to 1 if you have the `atanh' function. */
#define HAVE_ATANH 1

/* whether atof() accepts INF */
#define HAVE_ATOF_ACCEPTS_INF 1

/* whether atof() accepts NAN */
#define HAVE_ATOF_ACCEPTS_NAN 1

/* Define to 1 if you have the `atoll' function. */
#define HAVE_ATOLL 1

/* Whether you have bcmath */
#define HAVE_BCMATH 1

/* */
/* #undef HAVE_BIND_TEXTDOMAIN_CODESET */

/* */
/* #undef HAVE_BIRDSTEP */

/* Define if system has broken getcwd */
/* #undef HAVE_BROKEN_GETCWD */

/* Define if your glibc borks on fopen with mode a+ */
#define HAVE_BROKEN_GLIBC_FOPEN_APPEND 1

/* Whether we have librecode 3.5 */
/* #undef HAVE_BROKEN_RECODE */

/* Konstantin Chuguev's iconv implementation */
/* #undef HAVE_BSD_ICONV */

/* */
#define HAVE_BUILD_DEFS_H 1

/* */
#define HAVE_BUNDLED_PCRE 1

/* */
/* #undef HAVE_BZ2 */

/* */
/* #undef HAVE_CALENDAR */

/* Whether to compile with Caudium support */
/* #undef HAVE_CAUDIUM */

/* Define to 1 if you have the `chroot' function. */
#define HAVE_CHROOT 1

/* */
/* #undef HAVE_CLI0CLI_H */

/* */
/* #undef HAVE_CLI0CORE_H */

/* */
/* #undef HAVE_CLI0DEFS_H */

/* */
/* #undef HAVE_CLI0ENV_H */

/* */
/* #undef HAVE_CLI0EXT_H */

/* Whether you have struct cmsghdr */
#define HAVE_CMSGHDR 1

/* */
/* #undef HAVE_CODBC */

/* */
#define HAVE_COLORCLOSESTHWB 1

/* Whether you have a Continuity Server */
/* #undef HAVE_CONTINUITY */

/* Define to 1 if you have the `CreateProcess' function. */
/* #undef HAVE_CREATEPROCESS */

/* */
#define HAVE_CRYPT 1

/* Define to 1 if you have the <crypt.h> header file. */
#define HAVE_CRYPT_H 1

/* Define to 1 if you have the `crypt_r' function. */
/* #undef HAVE_CRYPT_R */

/* Define to 1 if you have the `ctermid' function. */
/* #undef HAVE_CTERMID */

/* Define to 1 if you have the `ctime_r' function. */
#define HAVE_CTIME_R 1

/* */
#define HAVE_CTYPE 1

/* */
#define HAVE_CURL 1

/* */
#define HAVE_CURL_EASY_STRERROR 1

/* Have cURL with GnuTLS support */
/* #undef HAVE_CURL_GNUTLS */

/* */
#define HAVE_CURL_MULTI_STRERROR 1

/* Have cURL with OpenSSL support */
#define HAVE_CURL_OPENSSL 1

/* Have cURL with SSL support */
#define HAVE_CURL_SSL 1

/* */
#define HAVE_CURL_VERSION_INFO 1

/* Define to 1 if you have the `cuserid' function. */
#define HAVE_CUSERID 1

/* */
/* #undef HAVE_DBA */

/* Whether you want DBMaker */
/* #undef HAVE_DBMAKER */

/* */
/* #undef HAVE_DCNGETTEXT */

/* Whether system headers declare timezone */
#define HAVE_DECLARED_TIMEZONE 1

/* Define to 1 if you have the <default_store.h> header file. */
/* #undef HAVE_DEFAULT_STORE_H */

/* */
/* #undef HAVE_DESTROY_SWF_BLOCK */

/* Define if the target system has /dev/urandom device */
#define HAVE_DEV_URANDOM 1

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* */
#define HAVE_DLOPEN 1

/* Whether you have dmalloc */
/* #undef HAVE_DMALLOC */

/* */
/* #undef HAVE_DNGETTEXT */

/* */
#define HAVE_DN_EXPAND 1

/* */
#define HAVE_DN_SKIPNAME 1

/* */
#define HAVE_DOM 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* OpenSSL 0.9.7 or later */
/* #undef HAVE_DSA_DEFAULT_METHOD */

/* embedded MySQL support enabled */
/* #undef HAVE_EMBEDDED_MYSQLI */

/* */
/* #undef HAVE_EMPRESS */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* */
/* #undef HAVE_ESOOB */

/* Whether you want EXIF (metadata from images) support */
#define HAVE_EXIF 1

/* Define to 1 if you have the `fabsf' function. */
#define HAVE_FABSF 1

/* Whether you have FrontBase */
/* #undef HAVE_FBSQL */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* */
/* #undef HAVE_FDFLIB */

/* */
/* #undef HAVE_FDFTK_5 */

/* */
/* #undef HAVE_FDFTK_H_LOWER */

/* Define to 1 if you have the `finite' function. */
#define HAVE_FINITE 1

/* Define to 1 if you have the `flock' function. */
#define HAVE_FLOCK 1

/* Define to 1 if you have the `floorf' function. */
#define HAVE_FLOORF 1

/* Define if flush should be called explicitly after a buffered io. */
/* #undef HAVE_FLUSHIO */

/* Define to 1 if your system has a working POSIX `fnmatch' function. */
#define HAVE_FNMATCH 1

/* */
#define HAVE_FOPENCOOKIE 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the `fpclass' function. */
/* #undef HAVE_FPCLASS */

/* whether floatingpoint.h defines fp_except */
/* #undef HAVE_FP_EXCEPT */

/* */
/* #undef HAVE_FREETDS */

/* Define to 1 if you have the `ftok' function. */
#define HAVE_FTOK 1

/* Whether you want FTP support */
/* #undef HAVE_FTP */

/* Define to 1 if you have the `funopen' function. */
/* #undef HAVE_FUNOPEN */

/* Define to 1 if you have the `gai_strerror' function. */
#define HAVE_GAI_STRERROR 1

/* Whether you have gcov */
/* #undef HAVE_GCOV */

/* Define to 1 if you have the `gcvt' function. */
#define HAVE_GCVT 1

/* */
#define HAVE_GDIMAGECOLORRESOLVE 1

/* */
#define HAVE_GD_BUNDLED 1

/* */
/* #undef HAVE_GD_CACHE_CREATE */

/* */
#define HAVE_GD_DYNAMIC_CTX_EX 1

/* */
#define HAVE_GD_FONTCACHESHUTDOWN 1

/* */
#define HAVE_GD_FONTMUTEX 1

/* */
/* #undef HAVE_GD_FREEFONTCACHE */

/* */
#define HAVE_GD_GD2 1

/* */
#define HAVE_GD_GIF_CREATE 1

/* */
#define HAVE_GD_GIF_CTX 1

/* */
#define HAVE_GD_GIF_READ 1

/* */
#define HAVE_GD_IMAGEELLIPSE 1

/* */
#define HAVE_GD_IMAGESETBRUSH 1

/* */
#define HAVE_GD_IMAGESETTILE 1

/* */
#define HAVE_GD_STRINGFT 1

/* */
#define HAVE_GD_STRINGFTEX 1

/* */
/* #undef HAVE_GD_STRINGTTF */

/* */
#define HAVE_GD_WBMP 1

/* */
#define HAVE_GD_XBM 1

/* */
/* #undef HAVE_GD_XPM */

/* Define if you have the getaddrinfo function */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `getgrgid_r' function. */
/* #undef HAVE_GETGRGID_R */

/* Define to 1 if you have the `getgrnam_r' function. */
#define HAVE_GETGRNAM_R 1

/* Define to 1 if you have the `getgroups' function. */
/* #undef HAVE_GETGROUPS */

/* */
#define HAVE_GETHOSTBYADDR 1

/* */
#define HAVE_GETHOSTBYNAME2 1

/* */
#define HAVE_GETHOSTNAME 1

/* Define to 1 if you have the `getloadavg' function. */
#define HAVE_GETLOADAVG 1

/* Define to 1 if you have the `getlogin' function. */
#define HAVE_GETLOGIN 1

/* Define to 1 if you have the `getopt' function. */
#define HAVE_GETOPT 1

/* Define to 1 if you have the `getpgid' function. */
/* #undef HAVE_GETPGID */

/* Define to 1 if you have the `getpid' function. */
#define HAVE_GETPID 1

/* Define to 1 if you have the `getpriority' function. */
/* #undef HAVE_GETPRIORITY */

/* Define to 1 if you have the `getprotobyname' function. */
#define HAVE_GETPROTOBYNAME 1

/* Define to 1 if you have the `getprotobynumber' function. */
#define HAVE_GETPROTOBYNUMBER 1

/* Define to 1 if you have the `getpwnam_r' function. */
#define HAVE_GETPWNAM_R 1

/* Define to 1 if you have the `getpwuid_r' function. */
#define HAVE_GETPWUID_R 1

/* Define to 1 if you have the `getrlimit' function. */
/* #undef HAVE_GETRLIMIT */

/* Define to 1 if you have the `getrusage' function. */
#define HAVE_GETRUSAGE 1

/* Define to 1 if you have the `getservbyname' function. */
#define HAVE_GETSERVBYNAME 1

/* Define to 1 if you have the `getservbyport' function. */
#define HAVE_GETSERVBYPORT 1

/* Define to 1 if you have the `getsid' function. */
/* #undef HAVE_GETSID */

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `getwd' function. */
#define HAVE_GETWD 1

/* */
/* #undef HAVE_GICONV_H */

/* glibc's iconv implementation */
#define HAVE_GLIBC_ICONV 1

/* Define to 1 if you have the `glob' function. */
#define HAVE_GLOB 1

/* */
/* #undef HAVE_GMP */

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define to 1 if you have the `grantpt' function. */
#define HAVE_GRANTPT 1

/* Define to 1 if you have the <grp.h> header file. */
#define HAVE_GRP_H 1

/* Have HASH Extension */
#define HAVE_HASH_EXT 1

/* Define to 1 if you have the `hstrerror' function. */
#define HAVE_HSTRERROR 1

/* */
#define HAVE_HTONL 1

/* whether HUGE_VAL == INF */
#define HAVE_HUGE_VAL_INF 1

/* whether HUGE_VAL + -HUGEVAL == NAN */
#define HAVE_HUGE_VAL_NAN 1

/* Define to 1 if you have the `hypot' function. */
#define HAVE_HYPOT 1

/* */
/* #undef HAVE_IBASE */

/* */
/* #undef HAVE_IBMDB2 */

/* */
#define HAVE_ICONV 1

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef HAVE_IEEEFP_H */

/* Define to 1 if you have the `if_indextoname' function. */
#define HAVE_IF_INDEXTONAME 1

/* Define to 1 if you have the `if_nametoindex' function. */
#define HAVE_IF_NAMETOINDEX 1

/* */
/* #undef HAVE_IMAP */

/* */
/* #undef HAVE_IMAP2000 */

/* */
/* #undef HAVE_IMAP2001 */

/* */
/* #undef HAVE_IMAP2004 */

/* */
/* #undef HAVE_IMAP_AUTH_GSS */

/* */
/* #undef HAVE_IMAP_KRB */

/* */
/* #undef HAVE_IMAP_SSL */

/* */
#define HAVE_INET_ATON 1

/* Define to 1 if you have the `inet_ntoa' function. */
#define HAVE_INET_NTOA 1

/* Define to 1 if you have the `inet_ntop' function. */
#define HAVE_INET_NTOP 1

/* Define to 1 if you have the `inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the `initgroups' function. */
/* #undef HAVE_INITGROUPS */

/* Define if int32_t type is present. */
#define HAVE_INT32_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* */
/* #undef HAVE_IODBC */

/* */
/* #undef HAVE_IODBC_H */

/* Whether to enable IPv6 support */
#define HAVE_IPV6 1

/* Define to 1 if you have the `isascii' function. */
#define HAVE_ISASCII 1

/* Define to 1 if you have the `isfinite' function. */
/* #undef HAVE_ISFINITE */

/* Define to 1 if you have the `isinf' function. */
#define HAVE_ISINF 1

/* Define to 1 if you have the `isnan' function. */
#define HAVE_ISNAN 1

/* */
/* #undef HAVE_ISQLEXT_H */

/* */
/* #undef HAVE_ISQL_H */

/* whether to enable JavaScript Object Serialization support */
#define HAVE_JSON 1

/* Define to 1 if you have the `kill' function. */
#define HAVE_KILL 1

/* Define to 1 if you have the <langinfo.h> header file. */
#define HAVE_LANGINFO_H 1

/* Define to 1 if you have the `lchown' function. */
#define HAVE_LCHOWN 1

/* */
/* #undef HAVE_LDAP */

/* Define to 1 if you have the `ldap_parse_reference' function. */
/* #undef HAVE_LDAP_PARSE_REFERENCE */

/* Define to 1 if you have the `ldap_parse_result' function. */
/* #undef HAVE_LDAP_PARSE_RESULT */

/* LDAP SASL support */
/* #undef HAVE_LDAP_SASL */

/* */
/* #undef HAVE_LDAP_SASL_H */

/* */
/* #undef HAVE_LDAP_SASL_SASL_H */

/* Define to 1 if you have the `ldap_start_tls_s' function. */
/* #undef HAVE_LDAP_START_TLS_S */

/* */
/* #undef HAVE_LIBBIND */

/* */
/* #undef HAVE_LIBCRYPT */

/* */
#define HAVE_LIBDL 1

/* */
/* #undef HAVE_LIBDNET_STUB */

/* */
/* #undef HAVE_LIBEDIT */

/* */
#define HAVE_LIBEXPAT 1

/* */
#define HAVE_LIBGD 1

/* */
#define HAVE_LIBGD13 1

/* */
#define HAVE_LIBGD15 1

/* */
#define HAVE_LIBGD20 1

/* */
#define HAVE_LIBGD204 1

/* */
/* #undef HAVE_LIBICONV */

/* */
/* #undef HAVE_LIBINTL */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* */
#define HAVE_LIBMCRYPT 1

/* */
/* #undef HAVE_LIBMHASH */

/* Whether you have libmm */
/* #undef HAVE_LIBMM */

/* */
#define HAVE_LIBNSL 1

/* */
/* #undef HAVE_LIBPAM */

/* */
/* #undef HAVE_LIBRARYMANAGER_H */

/* */
/* #undef HAVE_LIBREADLINE */

/* Whether we have librecode 3.5 or higher */
/* #undef HAVE_LIBRECODE */

/* */
#define HAVE_LIBRESOLV 1

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* */
/* #undef HAVE_LIBSOCKET */

/* */
/* #undef HAVE_LIBT1 */

/* */
/* #undef HAVE_LIBTTF */

/* */
#define HAVE_LIBXML 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `link' function. */
#define HAVE_LINK 1

/* Define to 1 if you have the `localeconv' function. */
#define HAVE_LOCALECONV 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `lockf' function. */
#define HAVE_LOCKF 1

/* Define to 1 if you have the `log1p' function. */
#define HAVE_LOG1P 1

/* Define to 1 if you have the `lrand48' function. */
#define HAVE_LRAND48 1

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
/* #undef HAVE_MACH_O_DYLD_H */

/* Define to 1 if you have the `makedev' function. */
/* #undef HAVE_MAKEDEV */

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the `mblen' function. */
#define HAVE_MBLEN 1

/* whether to have multibyte regex support */
/* #undef HAVE_MBREGEX */

/* Define to 1 if you have the `mbrlen' function. */
#define HAVE_MBRLEN 1

/* Define to 1 if you have the `mbsinit' function. */
#define HAVE_MBSINIT 1

/* Define if your system has mbstate_t in wchar.h */
#define HAVE_MBSTATE_T 1

/* whether to have multibyte string support */
/* #undef HAVE_MBSTRING */

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H */

/* Define if the target system has support for memory allocation using
   mmap(MAP_ANON) */
#define HAVE_MEM_MMAP_ANON 1

/* Define if the target system has support for memory allocation using
   mmap("/dev/zero") */
#define HAVE_MEM_MMAP_ZERO 1

/* */
/* #undef HAVE_MING */

/* */
/* #undef HAVE_MING_MOVIE_LEVEL */

/* */
/* #undef HAVE_MING_SETSWFCOMPRESSION */

/* */
/* #undef HAVE_MING_ZLIB */

/* Define to 1 if you have the `mkfifo' function. */
/* #undef HAVE_MKFIFO */

/* Define to 1 if you have the `mknod' function. */
/* #undef HAVE_MKNOD */

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP 1

/* Define to 1 if you have the <monetary.h> header file. */
#define HAVE_MONETARY_H 1

/* Define to 1 if you have the `mremap' function. */
#define HAVE_MREMAP 1

/* */
/* #undef HAVE_MSQL */

/* */
/* #undef HAVE_MSSQL */

/* Whether you have MySQL */
#define HAVE_MYSQL 1

/* */
/* #undef HAVE_MYSQLILIB */

/* Define to 1 if you have the `mysql_commit' function. */
/* #undef HAVE_MYSQL_COMMIT */

/* Define to 1 if you have the `mysql_next_result' function. */
/* #undef HAVE_MYSQL_NEXT_RESULT */

/* Define to 1 if you have the `mysql_sqlstate' function. */
/* #undef HAVE_MYSQL_SQLSTATE */

/* Define to 1 if you have the `mysql_stmt_prepare' function. */
/* #undef HAVE_MYSQL_STMT_PREPARE */

/* Define to 1 if you have the `nanosleep' function. */
#define HAVE_NANOSLEEP 1

/* */
/* #undef HAVE_NCURSESLIB */

/* */
/* #undef HAVE_NCURSES_ASSUME_DEFAULT_COLORS */

/* */
/* #undef HAVE_NCURSES_COLOR_SET */

/* */
/* #undef HAVE_NCURSES_H */

/* */
/* #undef HAVE_NCURSES_PANEL */

/* */
/* #undef HAVE_NCURSES_SLK_COLOR */

/* */
/* #undef HAVE_NCURSES_USE_EXTENDED_NAMES */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#define HAVE_NETINET_TCP_H 1

/* */
/* #undef HAVE_NET_SNMP */

/* Whether utf8_mime2text() has new signature */
/* #undef HAVE_NEW_MIME2TEXT */

/* */
/* #undef HAVE_NEW_MING */

/* */
/* #undef HAVE_NGETTEXT */

/* Define to 1 if you have the `nice' function. */
#define HAVE_NICE 1

/* Define to 1 if you have the `nl_langinfo' function. */
#define HAVE_NL_LANGINFO 1

/* Whether you have a Netscape/iPlanet/Sun Webserver */
/* #undef HAVE_NSAPI */

/* */
/* #undef HAVE_NSLDAP */

/* */
/* #undef HAVE_OCI8 */

/* */
/* #undef HAVE_OCI8_ATTR_STATEMENT */

/* */
/* #undef HAVE_OCI8_TEMP_LOB */

/* */
/* #undef HAVE_OCICOLLASSIGN */

/* */
/* #undef HAVE_OCIENVCREATE */

/* */
/* #undef HAVE_OCIENVNLSCREATE */

/* */
/* #undef HAVE_OCILOBISTEMPORARY */

/* */
/* #undef HAVE_OCISTMTFETCH2 */

/* */
/* #undef HAVE_OCI_ENV_CREATE */

/* */
/* #undef HAVE_OCI_ENV_NLS_CREATE */

/* */
/* #undef HAVE_OCI_INSTANT_CLIENT */

/* */
/* #undef HAVE_OCI_LOB_READ2 */

/* */
/* #undef HAVE_OCI_STMT_PREPARE2 */

/* */
/* #undef HAVE_ODBC2 */

/* */
/* #undef HAVE_ODBCSDK_H */

/* */
/* #undef HAVE_ODBC_H */

/* */
/* #undef HAVE_ODBC_ROUTER */

/* */
/* #undef HAVE_OLD_COMPAT_H */

/* whether you have old-style readdir_r */
/* #undef HAVE_OLD_READDIR_R */

/* */
/* #undef HAVE_OPENSSL_EXT */

/* */
/* #undef HAVE_ORALDAP */

/* */
/* #undef HAVE_ORALDAP_10 */

/* Whether struct _zend_object_value is packed */
#define HAVE_PACKED_OBJECT_VALUE 0

/* */
/* #undef HAVE_PCRE */

/* */
/* #undef HAVE_PDO_DBLIB */

/* */
/* #undef HAVE_PDO_FIREBIRD */

/* */
/* #undef HAVE_PDO_SQLITELIB */

/* Define to 1 if you have the `perror' function. */
#define HAVE_PERROR 1

/* */
/* #undef HAVE_PHP_SESSION */

/* Whether you have phttpd */
/* #undef HAVE_PHTTPD */

/* Define to 1 if you have the `poll' function. */
#define HAVE_POLL 1

/* whether to include POSIX-like functions */
/* #undef HAVE_POSIX */

/* whether you have POSIX readdir_r */
#define HAVE_POSIX_READDIR_R 1

/* PostgreSQL 7.0.x or later */
/* #undef HAVE_PQCLIENTENCODING */

/* Broken libpq under windows */
/* #undef HAVE_PQCMDTUPLES */

/* PostgreSQL 7.2.0 or later */
/* #undef HAVE_PQESCAPE */

/* PostgreSQL 8.1.4 or later */
/* #undef HAVE_PQESCAPE_BYTEA_CONN */

/* PostgreSQL 8.1.4 or later */
/* #undef HAVE_PQESCAPE_CONN */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQEXECPARAMS */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQEXECPREPARED */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQFREEMEM */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQFTABLE */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQGETCOPYDATA */

/* Older PostgreSQL */
/* #undef HAVE_PQOIDVALUE */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQPARAMETERSTATUS */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQPREPARE */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQPROTOCOLVERSION */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQPUTCOPYDATA */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQPUTCOPYEND */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQRESULTERRORFIELD */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQSENDPREPARE */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQSENDQUERYPARAMS */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQSENDQUERYPREPARED */

/* PostgreSQL 7.4 or later */
/* #undef HAVE_PQSETERRORVERBOSITY */

/* PostgreSQL 7.0.x or later */
/* #undef HAVE_PQSETNONBLOCKING */

/* PostgreSQL 7.3.0 or later */
/* #undef HAVE_PQUNESCAPEBYTEA */

/* */
/* #undef HAVE_PREAD */

/* */
/* #undef HAVE_PSPELL */

/* Define to 1 if you have the `ptsname' function. */
#define HAVE_PTSNAME 1

/* Define to 1 if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define to 1 if you have the <pwd.h> header file. */
#define HAVE_PWD_H 1

/* */
/* #undef HAVE_PWRITE */

/* Define to 1 if you have the `random' function. */
#define HAVE_RANDOM 1

/* Define to 1 if you have the `rand_r' function. */
#define HAVE_RAND_R 1

/* Define to 1 if you have the `realpath' function. */
#define HAVE_REALPATH 1

/* Whether Reflection is enabled */
#define HAVE_REFLECTION 1

/* Define to 1 if you have the `regcomp' function. */
#define HAVE_REGCOMP 1

/* 1 */
#define HAVE_REGEX_T_RE_MAGIC 1

/* Define to 1 if you have the <resolv.h> header file. */
#define HAVE_RESOLV_H 1

/* */
#define HAVE_RES_NMKQUERY 1

/* */
#define HAVE_RES_NSEND 1

/* Define to 1 if you have the `res_search' function. */
#define HAVE_RES_SEARCH 1

/* */
/* #undef HAVE_RL_CALLBACK_READ_CHAR */

/* Define to 1 if you have the `rl_completion_matches' function. */
/* #undef HAVE_RL_COMPLETION_MATCHES */

/* Whether you use Roxen */
/* #undef HAVE_ROXEN */

/* */
/* #undef HAVE_SAPDB */

/* Define to 1 if you have the `scandir' function. */
#define HAVE_SCANDIR 1

/* */
#define HAVE_SEMUN 0

/* whether you have sendmail */
#define HAVE_SENDMAIL 1

/* Define to 1 if you have the `setegid' function. */
/* #undef HAVE_SETEGID */

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `seteuid' function. */
/* #undef HAVE_SETEUID */

/* Define to 1 if you have the `setitimer' function. */
#define HAVE_SETITIMER 1

/* Define to 1 if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define to 1 if you have the `setpgid' function. */
#define HAVE_SETPGID 1

/* Define to 1 if you have the `setpriority' function. */
/* #undef HAVE_SETPRIORITY */

/* Define to 1 if you have the `setsid' function. */
/* #undef HAVE_SETSID */

/* Define to 1 if you have the `setsockopt' function. */
#define HAVE_SETSOCKOPT 1

/* Define to 1 if you have the `setvbuf' function. */
#define HAVE_SETVBUF 1

/* */
/* #undef HAVE_SHMOP */

/* Define to 1 if you have the `shutdown' function. */
#define HAVE_SHUTDOWN 1

/* */
/* #undef HAVE_SIGACTION */

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* */
#define HAVE_SIMPLEXML 1

/* Define to 1 if you have the `sin' function. */
#define HAVE_SIN 1

/* */
/* #undef HAVE_SNMP */

/* */
/* #undef HAVE_SNMP_PARSE_OID */

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* */
#define HAVE_SOAP 1

/* Whether struct sockaddr has field sa_len */
/* #undef HAVE_SOCKADDR_SA_LEN */

/* Whether you have struct sockaddr_storage */
#define HAVE_SOCKADDR_STORAGE 1

/* */
#define HAVE_SOCKET 1

/* */
#define HAVE_SOCKETPAIR 1

/* */
#define HAVE_SOCKETS 1

/* Whether you have socklen_t */
#define HAVE_SOCKLEN_T 1

/* */
/* #undef HAVE_SOLID */

/* */
/* #undef HAVE_SOLID_30 */

/* */
/* #undef HAVE_SOLID_35 */

/* Whether you want SPL (Standard PHP Library) support */
#define HAVE_SPL 1

/* */
/* #undef HAVE_SQLCLI1_H */

/* */
/* #undef HAVE_SQLDATASOURCES */

/* */
/* #undef HAVE_SQLEXT_H */

/* have commercial sqlite3 with crypto support */
/* #undef HAVE_SQLITE3_KEY */

/* */
/* #undef HAVE_SQLTYPES_H */

/* */
/* #undef HAVE_SQLUCODE_H */

/* */
/* #undef HAVE_SQLUNIX_H */

/* */
/* #undef HAVE_SQL_H */

/* Define to 1 if you have the `srand48' function. */
#define HAVE_SRAND48 1

/* Define to 1 if you have the `srandom' function. */
#define HAVE_SRANDOM 1

/* Define to 1 if you have the `statfs' function. */
#define HAVE_STATFS 1

/* Define to 1 if you have the `statvfs' function. */
#define HAVE_STATVFS 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define if stdarg.h is available */
/* #undef HAVE_STDARG_PROTOTYPES */

/* Define to 1 if you have the <stdbool.h> header file. */
/* #undef HAVE_STDBOOL_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `std_syslog' function. */
/* #undef HAVE_STD_SYSLOG */

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcoll' function. */
#define HAVE_STRCOLL 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strfmon' function. */
#define HAVE_STRFMON 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strpbrk' function. */
#define HAVE_STRPBRK 1

/* Define to 1 if you have the `strptime' function. */
#define HAVE_STRPTIME 1

/* whether strptime() declaration fails */
/* #undef HAVE_STRPTIME_DECL_FAILS */

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the `strtod' function. */
#define HAVE_STRTOD 1

/* Define to 1 if you have the `strtok_r' function. */
#define HAVE_STRTOK_R 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the `strtoll' function. */
#define HAVE_STRTOLL 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the `strtoull' function. */
#define HAVE_STRTOULL 1

/* whether you have struct flock */
#define HAVE_STRUCT_FLOCK 1

/* Define to 1 if `st_blksize' is member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_BLKSIZE */

/* Define to 1 if `st_blocks' is member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_BLOCKS */

/* Define to 1 if `st_rdev' is member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_RDEV */

/* Define to 1 if `tm_zone' is member of `struct tm'. */
#define HAVE_STRUCT_TM_TM_ZONE 1

/* Define to 1 if your `struct stat' has `st_blksize'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLKSIZE' instead. */
/* #undef HAVE_ST_BLKSIZE */

/* Define to 1 if your `struct stat' has `st_blocks'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLOCKS' instead. */
/* #undef HAVE_ST_BLOCKS */

/* Define to 1 if you have the <st.h> header file. */
/* #undef HAVE_ST_H */

/* Define to 1 if your `struct stat' has `st_rdev'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_RDEV' instead. */
/* #undef HAVE_ST_RDEV */

/* */
/* #undef HAVE_SWFMOVIE_NAMEDANCHOR */

/* */
/* #undef HAVE_SWFPREBUILTCLIP */

/* */
/* #undef HAVE_SYBASE */

/* */
/* #undef HAVE_SYBASE_CT */

/* Define to 1 if you have the `symlink' function. */
#define HAVE_SYMLINK 1

/* Define to 1 if you have the <sysexits.h> header file. */
#define HAVE_SYSEXITS_H 1

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* */
/* #undef HAVE_SYSVMSG */

/* */
/* #undef HAVE_SYSVSEM */

/* */
/* #undef HAVE_SYSVSHM */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/file.h> header file. */
#define HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ipc.h> header file. */
#define HAVE_SYS_IPC_H 1

/* Define to 1 if you have the <sys/loadavg.h> header file. */
/* #undef HAVE_SYS_LOADAVG_H */

/* Define to 1 if you have the <sys/mkdev.h> header file. */
/* #undef HAVE_SYS_MKDEV_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
#define HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/mount.h> header file. */
#define HAVE_SYS_MOUNT_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/poll.h> header file. */
#define HAVE_SYS_POLL_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/statfs.h> header file. */
#define HAVE_SYS_STATFS_H 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/sysexits.h> header file. */
/* #undef HAVE_SYS_SYSEXITS_H */

/* Define to 1 if you have the <sys/times.h> header file. */
/* #undef HAVE_SYS_TIMES_H */

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#define HAVE_SYS_UN_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <sys/varargs.h> header file. */
/* #undef HAVE_SYS_VARARGS_H */

/* Define to 1 if you have the <sys/vfs.h> header file. */
#define HAVE_SYS_VFS_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the `tempnam' function. */
#define HAVE_TEMPNAM 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* */
/* #undef HAVE_TIDY */

/* */
/* #undef HAVE_TIDYOPTGETDOC */

/* Define to 1 if you have the <time.h> header file. */
/* #undef HAVE_TIME_H */

/* whether you have tm_gmtoff in struct tm */
#define HAVE_TM_GMTOFF 1

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#define HAVE_TM_ZONE 1

/* Whether you have a working ttyname_r */
/* #undef HAVE_TTYNAME_R */

/* Define to 1 if you have the <tuxmodule.h> header file. */
/* #undef HAVE_TUXMODULE_H */

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef HAVE_TZNAME */

/* Define to 1 if you have the `tzset' function. */
#define HAVE_TZSET 1

/* */
/* #undef HAVE_UDBCEXT_H */

/* Define if uint32_t type is present. */
#define HAVE_UINT32_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* */
/* #undef HAVE_UNIXODBC */

/* Define to 1 if you have the <unix.h> header file. */
/* #undef HAVE_UNIX_H */

/* Define to 1 if you have the `unlockpt' function. */
#define HAVE_UNLOCKPT 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* */
/* #undef HAVE_UODBC */

/* Define to 1 if you have the `usleep' function. */
#define HAVE_USLEEP 1

/* Define to 1 if you have the `utime' function. */
#define HAVE_UTIME 1

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1

/* Define to 1 if `utime(file, NULL)' sets file's timestamp to the present. */
/* #undef HAVE_UTIME_NULL */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `wait3' function. */
/* #undef HAVE_WAIT3 */

/* */
/* #undef HAVE_WAITPID */

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* */
#define HAVE_XML 1

/* Define to 1 if you have the <xmlparse.h> header file. */
/* #undef HAVE_XMLPARSE_H */

/* */
#define HAVE_XMLREADER 1

/* */
#define HAVE_XMLRPC 1

/* Define to 1 if you have the <xmltok.h> header file. */
/* #undef HAVE_XMLTOK_H */

/* */
#define HAVE_XMLWRITER 1

/* */
/* #undef HAVE_XSL */

/* */
/* #undef HAVE_XSL_EXSLT */

/* */
#define HAVE_YP_GET_DEFAULT_DOMAIN 1

/* */
/* #undef HAVE_ZIP */

/* */
#define HAVE_ZLIB 1

/* */
/* #undef HPUX */

/* */
#define HSREGEX 1

/* Whether iconv supports error no or not */
#define ICONV_SUPPORTS_ERRNO 1

/* */
/* #undef ISOLARIS */

/* */
/* #undef LINUX */

/* */
#define MAGIC_QUOTES 0

/* Whether asctime_r is declared */
/* #undef MISSING_ASCTIME_R_DECL */

/* Whether ctime_r is declared */
/* #undef MISSING_CTIME_R_DECL */

/* */
#define MISSING_FCLOSE_DECL 0

/* Whether gmtime_r is declared */
/* #undef MISSING_GMTIME_R_DECL */

/* Whether localtime_r is declared */
/* #undef MISSING_LOCALTIME_R_DECL */

/* */
/* #undef MISSING_MSGHDR_MSGFLAGS */

/* Whether strtok_r is declared */
/* #undef MISSING_STRTOK_R_DECL */

/* */
/* #undef MSQL1 */

/* */
/* #undef MYSQL_UNIX_ADDR */

/* */
/* #undef NDBM_INCLUDE_FILE */

/* */
/* #undef NEUTRINO */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
//#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
//#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
//#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
//#define PACKAGE_VERSION ""

/* */
/* #undef PDO_MYSQL_UNIX_ADDR */

/* */
#define PHP_APACHE_HAVE_CLIENT_FD 1

/* Whether the system supports BlowFish salt */
#define PHP_BLOWFISH_CRYPT 0

/* PHP build date */
#define PHP_BUILD_DATE "2008-01-09"

/* Define if your system has fork/vfork/CreateProcess */
#define PHP_CAN_SUPPORT_PROC_OPEN 1

/* */
/* #undef PHP_CURL_URL_WRAPPERS */

/* Whether the system supports extended DES salt */
#define PHP_EXT_DES_CRYPT 0

/* */
/* #undef PHP_FASTCGI */

/* Whether you have HP-UX 10.x */
/* #undef PHP_HPUX_TIME_R */

/* Path to iconv.h */
#define PHP_ICONV_H_PATH </usr/include/iconv.h>

/* Which iconv implementation to use */
#define PHP_ICONV_IMPL "glibc"

/* Whether you have IRIX-style functions */
/* #undef PHP_IRIX_TIME_R */

/* Whether the system supports MD5 salt */
#define PHP_MD5_CRYPT 1

/* magic file path */
/* #undef PHP_MIME_MAGIC_FILE_PATH */

/* */
/* #undef PHP_OCI8_HAVE_COLLECTIONS */

/* uname output */
#define PHP_OS "Linux"

/* whether pread64 is default */
/* #undef PHP_PREAD_64 */

/* whether pwrite64 is default */
/* #undef PHP_PWRITE_64 */

/* */
#define PHP_SAFE_MODE 0

/* */
#define PHP_SAFE_MODE_EXEC_DIR "/usr/local/php/bin"

/* */
#define PHP_SIGCHILD 0

/* Have PDO */
/* #undef PHP_SQLITE2_HAVE_PDO */

/* Whether the system supports standard DES salt */
#define PHP_STD_DES_CRYPT 1

/* */
/* #undef PHP_SYBASE_DBOPEN */

/* whether write(2) works */
#define PHP_WRITE_STDOUT 1

/* Whether to use Pthreads */
/* #undef PTHREADS */

/* */
/* #undef QDBM_INCLUDE_FILE */

/* */
#define REGEX 1

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Whether to use Roxen in ZTS mode */
/* #undef ROXEN_USE_ZTS */

/* The size of a `char', as computed by sizeof. */
#define SIZEOF_CHAR 1

/* The size of a `char *', as computed by sizeof. */
/* #undef SIZEOF_CHAR_P */

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `intmax_t', as computed by sizeof. */
#define SIZEOF_INTMAX_T 8

#define WORDSIZE_IS_64

/* The size of a `long', as computed by sizeof. */
#ifdef WORDSIZE_IS_64
#define SIZEOF_LONG 8
#else
#define SIZEOF_LONG 4
#endif

/* The size of a `long int', as computed by sizeof. */
/* #undef SIZEOF_LONG_INT */

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of a `long long int', as computed by sizeof. */
#define SIZEOF_LONG_LONG_INT 8

/* The size of a `ptrdiff_t', as computed by sizeof. */
#ifdef WORDSIZE_IS_64
#define SIZEOF_PTRDIFF_T 8
#else
#define SIZEOF_PTRDIFF_T 4
#endif

/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of a `size_t', as computed by sizeof. */
#ifdef WORDSIZE_IS_64
#define SIZEOF_SIZE_T 8
#else
#define SIZEOF_SIZE_T 4
#endif

/* The size of a `ssize_t', as computed by sizeof. */
#ifdef WORDSIZE_IS_64
#define SIZEOF_SSIZE_T 8
#else
#define SIZEOF_SSIZE_T 4
#endif

/* */
/* #undef SOLARIS */

/* Size of a pointer */
/* #undef SQLITE_PTR_SZ */

/* */
/* #undef SQLITE_UTF8 */

/* Needed in sqlunix.h for wchar defs */
/* #undef SS_FBX */

/* Needed in sqlunix.h */
/* #undef SS_LINUX */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
   STACK_DIRECTION > 0 => grows toward higher addresses
   STACK_DIRECTION < 0 => grows toward lower addresses
   STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* */
/* #undef TSRM_ST */

/* */
/* #undef UCD_SNMP_HACK */

/* */
#define UNDEF_THREADS_HACK

/* */
/* #undef UNIXWARE */

/* whether to check multibyte regex backtrack */
/* #undef USE_COMBINATION_EXPLOSION_CHECK */

/* */
/* #undef USE_GD_JISX0208 */

/* */
/* #undef USE_TRANSFER_TABLES */

/* whether you want Pi3Web support */
/* #undef WITH_PI3WEB */

/* */
/* #undef WITH_ZEUS */

/* Define if processor uses big-endian word */
/* #undef WORDS_BIGENDIAN */

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#define YYTEXT_POINTER 1

/* Whether sprintf is broken */
#define ZEND_BROKEN_SPRINTF 0

/* */
#define ZEND_DEBUG 0

/* */
#define ZEND_MM_ALIGNMENT 8

/* */
#define ZEND_MM_ALIGNMENT_LOG2 3

/* */
/* #undef ZEND_MULTIBYTE */

/* virtual machine dispatch method */
#define ZEND_VM_KIND ZEND_VM_KIND_CALL

/* */
/* #undef ZTS */

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* # undef _ALL_SOURCE */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* */
/* #undef in_addr_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define to `unsigned int ' if <sys/types.h> does not define. */
/* #undef uint */

/* Define to `unsigned long ' if <sys/types.h> does not define. */
/* #undef ulong */

#ifndef ZEND_ACCONFIG_H_NO_C_PROTOS

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#if ZEND_BROKEN_SPRINTF
int zend_sprintf(char *buffer, const char *format, ...);
#else
# define zend_sprintf sprintf
#endif

#include <math.h>

/* To enable the is_nan, is_infinite and is_finite PHP functions */
#ifdef NETWARE
  #define HAVE_ISNAN 1
  #define HAVE_ISINF 1
  #define HAVE_ISFINITE 1
#endif

#ifndef zend_isnan
#ifdef HAVE_ISNAN
#define zend_isnan(a) isnan(a)
#elif defined(HAVE_FPCLASS)
#define zend_isnan(a) ((fpclass(a) == FP_SNAN) || (fpclass(a) == FP_QNAN))
#else
#define zend_isnan(a) 0
#endif
#endif

#ifdef HAVE_ISINF
#define zend_isinf(a) isinf(a)
#elif defined(INFINITY)
/* Might not work, but is required by ISO C99 */
#define zend_isinf(a) (((a)==INFINITY)?1:0)
#elif defined(HAVE_FPCLASS)
#define zend_isinf(a) ((fpclass(a) == FP_PINF) || (fpclass(a) == FP_NINF))
#else
#define zend_isinf(a) 0
#endif

#ifdef HAVE_FINITE
#define zend_finite(a) finite(a)
#elif defined(HAVE_ISFINITE) || defined(isfinite)
#define zend_finite(a) isfinite(a)
#elif defined(fpclassify)
#define zend_finite(a) ((fpclassify((a))!=FP_INFINITE&&fpclassify((a))!=FP_NAN)?1:0)
#else
#define zend_finite(a) (zend_isnan(a) ? 0 : zend_isinf(a) ? 0 : 1)
#endif

#endif /* ifndef ZEND_ACCONFIG_H_NO_C_PROTOS */

#ifdef NETWARE
#ifdef USE_WINSOCK
#/*This detection against winsock is of no use*/ undef HAVE_SOCKLEN_T
#/*This detection against winsock is of no use*/ undef HAVE_SYS_SOCKET_H
#endif
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
/* #undef PTHREADS */
