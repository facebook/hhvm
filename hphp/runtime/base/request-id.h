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
#pragma once

#include <string>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct RequestId {
  RequestId(): m_id(0) {}
  static RequestId allocate();
  std::string toString() const;
  int64_t id() const { return m_id; }
  bool unallocated() const { return m_id == 0; }
  bool operator==(RequestId o) const { return m_id == o.m_id; }
private:
  int64_t m_id;
};

//////////////////////////////////////////////////////////////////////

}
