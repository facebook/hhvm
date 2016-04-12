/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_STRING_HOLDER_H_
#define incl_HPHP_STRING_HOLDER_H_

#include <folly/io/IOBuf.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * String holder class for storing a reference to a string that is not
 * owned by us. The string may be either a (char *, len) pair, or
 * folly::IOBuf.
 *
 * If it is a (char*, len) pair, the char* is not freed on destruction.
 */
struct StringHolder {

  enum Type { StrFree, StrNoFree, IOBuf };

  explicit StringHolder(const char* data, uint32_t len, bool free = false);
  StringHolder(StringHolder&&);

  ~StringHolder();

  StringHolder& operator=(StringHolder&&);

  /* Set data source */
  void set(folly::IOBuf *output);

  uint32_t size() const;
  const char* data() const;

private:
  const char* m_data;
  uint32_t m_len;
  Type m_type;
  std::unique_ptr<folly::IOBuf> m_output;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_HOLDER_H_
