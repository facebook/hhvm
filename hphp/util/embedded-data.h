/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_UTIL_EMBEDDED_DATA_H_
#define incl_HPHP_UTIL_EMBEDDED_DATA_H_

#ifdef __cplusplus

#include <cstdint>
#include <string>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct embedded_data {
  std::string m_filename;
  uint64_t m_start;
  uint64_t m_len;
#ifdef _MSC_VER
  void* m_handle;
#endif
};

bool get_embedded_data(const char* section, embedded_data* desc,
                       const std::string& filename = "");

std::string read_embedded_data(const embedded_data& desc);

/*
 * dlopen() the embedded shared object given by `desc'.
 *
 * Unfortunately, there's no way to do the equivalent of dlopen() on data
 * within another file, or even in memory.  This means we have to copy the
 * section into a temporary file and then dlopen() that.
 *
 * Returns the result of dlopen() on success, else nullptr.  Also logs the
 * failure condition with Logger on failure.
 */
void* dlopen_embedded_data(const embedded_data& desc, char* tmp_filename);

/*
 * Clean up any /tmp files that we created at process shutdown time.
 */
void embedded_data_cleanup();


///////////////////////////////////////////////////////////////////////////////

}

#else //__cplusplus

/*
 * Read data from the named section and place it into the given buffer (of size
 * len) Returns the number of bytes (not null-terminated) read or -1 if
 * unsuccessful
 */
ssize_t hphp_read_embedded_data(const char* section, char* buf, size_t len);

#endif //__cplusplus

#endif
