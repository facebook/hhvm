/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __GENERATED_SYS_SYSTEM_GLOBALS_H__
#define __GENERATED_SYS_SYSTEM_GLOBALS_H__

#include <php/classes/arrayaccess.h>
#include <php/classes/exception.h>
#include <php/classes/iterator.h>
#include <php/classes/pear_error.h>
#include <php/classes/reflection.h>
#include <php/classes/splobjectstorage.h>
#include <php/classes/stdclass.h>
#include <php/globals/constants.h>
#include <php/globals/symbols.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


// Class Forward Declarations

class SystemGlobals : public Globals {
public:
  SystemGlobals();
  static void initialize();

  bool dummy; // for easier constructor initializer output

  // Global Variables
  Variant gv_argc;
  Variant gv_argv;
  Variant gv__SERVER;
  Variant gv__GET;
  Variant gv__POST;
  Variant gv__COOKIE;
  Variant gv__FILES;
  Variant gv__ENV;
  Variant gv__REQUEST;
  Variant gv__SESSION;
  Variant gv_HTTP_RAW_POST_DATA;
  Variant gv_http_response_header;

  // Dynamic Constants

  // Function/Method Static Variables

  // Function/Method Static Variable Initialization Booleans

  // Class Static Variables

  // Class Static Initializer Flags

  // PseudoMain Variables
  bool run_pm_php$classes$arrayaccess_php;
  bool run_pm_php$classes$exception_php;
  bool run_pm_php$classes$iterator_php;
  bool run_pm_php$classes$pear_error_php;
  bool run_pm_php$classes$reflection_php;
  bool run_pm_php$classes$splobjectstorage_php;
  bool run_pm_php$classes$stdclass_php;
  bool run_pm_php$globals$constants_php;
  bool run_pm_php$globals$symbols_php;

  // Redeclared Functions

  // Redeclared Classes
};

// Scalar Arrays
class SystemScalarArrays {
public:
  static void initialize();

  static StaticArray ssa_[1];
};

extern const int64 k_UCOL_CASE_LEVEL;
extern const StaticString k_MCC_ARG_POLL_TMO;
extern const int64 k_MCC_PROXY_GET_OP;
extern const StaticString k_MCC_ARG_CONN_NTRIES;
extern const int64 k_SQLITE3_TEXT;
extern const int64 k_MCC_SERVER_UP;
extern const int64 k_UCOL_CASE_FIRST;
extern const StaticString k_MCC_ARG_PROXY_OPS;
extern const int64 k_UCOL_DEFAULT_STRENGTH;
extern const int64 k_MCC_WINDOW_MAX;
extern const int64 k_SQLITE3_OPEN_CREATE;
extern const int64 k_FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
extern const int64 k_UCOL_ON;
extern const int64 k_MCC_TMO_MS;
extern const int64 k_MCC_POOLPREFIX_LEN;
extern const int64 k_SQLITE3_FLOAT;
extern const double k_MCC_DGRAM_TMO_WEIGHT;
extern const int64 k_MCC_POLL_TMO_US;
extern const StaticString k_MCC_ARG_COMPRESSION_THRESHOLD;
extern const StaticString k_MCC_ARG_NZLIB_COMPRESSION;
extern const StaticString k_MCC_ARG_DEBUG_LOGFILE;
extern const int64 k_UCOL_SHIFTED;
extern const int64 k_FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;
extern const int64 k_UCOL_IDENTICAL;
extern const int64 k_SQLITE3_OPEN_READWRITE;
extern const int64 k_MCC_HAVE_DEBUG_LOG;
extern const int64 k_MCC_SERVER_RETRY_TMO_MS;
extern const int64 k_MCC_CONSISTENCY_MATCH_HITS;
extern const int64 k_SQLITE3_ASSOC;
extern const int64 k_MCC_CONN_TMO_MS;
extern const int64 k_SQLITE3_NUM;
extern const int64 k_PHPMCC_NEW_HANDLE;
extern const int64 k_UCOL_HIRAGANA_QUATERNARY_MODE;
extern const int64 k_UCOL_STRENGTH;
extern const int64 k_MCC_UDP_REPLY_PORTS;
extern const int64 k_UCOL_TERTIARY;
extern const StaticString k_MCC_ARG_DELETE_PROXY;
extern const StaticString k_MCC_ARG_DGRAM_NTRIES;
extern const int64 k_MCC_PROXY_ARITH_OP;
extern const int64 k_PHPMCC_USED_SLOW_PATH;
extern const int64 k_MCC_GET_RECORD_ERRORS;
extern const int64 k_PHPMCC_USED_FAST_PATH;
extern const StaticString k_MCC_ARG_SERVERS;
extern const int64 k_MCC_PORT_DEFAULT;
extern const StaticString k_MCC_ARG_CONSISTENT_HASHING_PREFIXES;
extern const StaticString k_MCC_ARG_TCP_INACTIVITY_TIME;
extern const StaticString k_MCC_ARG_DGRAM_TMO_THRESHOLD;
extern const int64 k_FB_UNSERIALIZE_NONSTRING_VALUE;
extern const int64 k_UCOL_UPPER_FIRST;
extern const StaticString k_MCC_ARG_DGRAM_TMO_WEIGHT;
extern const int64 k_SQLITE3_INTEGER;
extern const int64 k_MCC_TCP_INACTIVITY_TMO_DEFAULT;
extern const int64 k_MCC_COMPRESSION_THRESHHOLD;
extern const StaticString k_MCC_ARG_MIRROR_CFG_NAME;
extern const StaticString k_MCC_ARG_MIRROR_CFG_MODEL;
extern const int64 k_SQLITE3_BOTH;
extern const int64 k_MCC_RXDGRAM_MAX;
extern const StaticString k_MCC_ARG_MIRROR_CFG;
extern const int64 k_MCC_SERVER_DISABLED;
extern const int64 k_UCOL_LOWER_FIRST;
extern const int64 k_UCOL_SECONDARY;
extern const int64 k_MCC_PROXY_UPDATE_OP;
extern const StaticString k_MCC_ARG_WINDOW_MAX;
extern const int64 k_MCC_IPPROTO_TCP;
extern const StaticString k_MCC_ARG_UDP_REPLY_PORTS;
extern const StaticString k_MCC_ARG_CONN_TMO;
extern const int64 k_MCC_HAVE_FB_SERIALIZATION;
extern const int64 k_UCOL_ALTERNATE_HANDLING;
extern const int64 k_SQLITE3_OPEN_READONLY;
extern const int64 k_MCC_CONSISTENCY_MATCH_ALL;
extern const StaticString k_MCC_ARG_PERSISTENT;
extern const int64 k_UCOL_NUMERIC_COLLATION;
extern const StaticString k_PHPMCC_VERSION;
extern const int64 k_MCC_DELETE_ERROR_LOG;
extern const int64 k_UCOL_NORMALIZATION_MODE;
extern const int64 k_MCC_CONSISTENCY_MATCH_HITS_SUPERCEDES;
extern const StaticString k_MCC_ARG_NPOOLPREFIX;
extern const StaticString k_MCC_ARG_TMO;
extern const StaticString k_MCC_ARG_NODELAY;
extern const StaticString k_MCC_ARG_MIRROR_CFG_SERVERPOOLS;
extern const int64 k_MCC_DELETE_ERROR_NOLOG;
extern const int64 k_MCC_CONN_NTRIES;
extern const int64 k_MCC_DGRAM_NTRIES;
extern const int64 k_MCC_NODELAY;
extern const int64 k_UCOL_DEFAULT;
extern const StaticString k_MCC_ARG_FB_SERIALIZE_ENABLED;
extern const int64 k_MCC_IPPROTO_UDP;
extern const StaticString k_MCC_ARG_DEBUG;
extern const int64 k_MCC_MTU;
extern const StaticString k_MCC_ARG_DEFAULT_PREFIX;
extern const int64 k_UCOL_FRENCH_COLLATION;
extern const int64 k_MCC_CONSISTENCY_IGNORE;
extern const int64 k_UCOL_PRIMARY;
extern const int64 k_MCC_DGRAM_TMO_THRESHOLD;
extern const int64 k_UCOL_OFF;
extern const int64 k_MCC_SERVER_DOWN;
extern const int64 k_UCOL_QUATERNARY;
extern const int64 k_MCC_DELETE_DELETED;
extern const StaticString k_MCC_ARG_PROXY;
extern const int64 k_SQLITE3_NULL;
extern const int64 k_MCC_PROXY_DELETE_OP;
extern const int64 k_MCC_DELETE_NOTFOUND;
extern const StaticString k_MCC_ARG_SERVER_RETRY_TMO_MS;
extern const int64 k_UCOL_NON_IGNORABLE;
extern const int64 k_FB_UNSERIALIZE_UNEXPECTED_END;
extern const int64 k_SQLITE3_BLOB;
extern const int64 k_MCC_HAVE_ZLIB_COMPRESSION;


///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_SYS_SYSTEM_GLOBALS_H__
