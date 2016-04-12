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

#include "hphp/runtime/base/string-holder.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

StringHolder::StringHolder(const char* data, uint32_t len, bool free)
  : m_data(data), m_len(len),
    m_type(free ? Type::StrFree : Type::StrNoFree),
    m_output(nullptr)
{}

StringHolder::StringHolder(StringHolder&& o)
  : m_len(o.m_len), m_type(o.m_type), m_output(std::move(o.m_output))
{
  m_data = o.m_data;
  o.m_data = nullptr;
}

StringHolder::~StringHolder() {
  if (m_type == Type::StrFree && m_data) free((void *)m_data);
}

StringHolder& StringHolder::operator=(StringHolder&& o) {
  m_type = o.m_type;
  m_output = std::move(o.m_output);
  m_len = o.m_len;
  m_data = o.m_data;
  o.m_data = nullptr;
  return *this;
}

void StringHolder::set(folly::IOBuf *output) {
  m_output = std::unique_ptr<folly::IOBuf>(output);
  m_type = Type::IOBuf;
}

uint32_t StringHolder::size() const {
  if (m_output) return m_output->length();
  return m_len;
}

const char* StringHolder::data() const {
  if (m_output) {
    return reinterpret_cast<const char*>(m_output->data());
  }
  return m_data;
}

///////////////////////////////////////////////////////////////////////////////

}
