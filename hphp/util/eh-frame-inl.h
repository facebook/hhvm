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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline const uint8_t* EHFrameDesc::Entry::get() const {
  return &m_vec[m_idx];
}

inline uint32_t EHFrameDesc::Entry::length() const {
  return *(uint32_t*)(&m_vec[m_idx]);
}

inline uint32_t EHFrameDesc::Entry::cie_off() const {
  return *(uint32_t*)(&m_vec[m_idx + sizeof(uint32_t)]);
}

inline CodeAddress EHFrameDesc::FDE::start() const {
  auto constexpr off = sizeof(uint32_t) + sizeof(uint32_t);
  return *(CodeAddress*)(&m_vec[m_idx + off]);
}

inline size_t EHFrameDesc::FDE::range() const {
  auto constexpr off = sizeof(uint32_t) + sizeof(uint32_t) +
                       sizeof(CodeAddress);
  return *(size_t*)(&m_vec[m_idx + off]);
}

///////////////////////////////////////////////////////////////////////////////

inline EHFrameDesc::CIE EHFrameDesc::cie() const {
  assertx(m_buf != nullptr);
  return CIE(*m_buf, 0);
}

template <class F>
inline void EHFrameDesc::for_each_fde(F f) const {
  assertx(m_buf != nullptr);

  auto const cie_len = cie().length();
  auto idx = sizeof(cie_len) + cie_len;

  while (idx < m_buf->size()) {
    auto fde = FDE(*m_buf, idx);
    auto const fde_len = fde.length();

    if (fde_len != 0) f(fde); // skip the null-terminating FDE

    idx += sizeof(fde_len) + fde_len;
    assertx(fde_len != 0 || idx == m_buf->size());
  }
}

///////////////////////////////////////////////////////////////////////////////

}
