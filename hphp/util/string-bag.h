/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_STRING_BAG_H_
#define incl_HPHP_STRING_BAG_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Holding a list of char* safely, so other places only need to store char*
 * than making a copy.
 */
class StringBag {
public:
  explicit StringBag(int reserve_count = 0);
  ~StringBag();

  const char *add(const char *s);
  unsigned int size() const { return m_strings.size();}
  const char *at(unsigned int index) const;

private:
  std::vector<char *> m_strings;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_BAG_H_
