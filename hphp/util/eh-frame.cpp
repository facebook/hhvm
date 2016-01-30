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

#include "hphp/util/eh-frame.h"

#include "hphp/util/assertions.h"

#include <folly/ScopeGuard.h>

#include <cstring>
#include <memory>
#include <vector>

#if defined(__CYGWIN__) || defined(_MSC_VER)
void __register_frame(const void*) {}
void __deregister_frame(const void*) {}
#else
/*
 * libgcc exports these for registering EH information for dynamically-loaded
 * objects.  The argument is a pointer to data in the format you find in an
 * .eh_frame section.
 */
extern "C" void __register_frame(const void*);
extern "C" void __deregister_frame(const void*);
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

void EHFrameWriter::write_uleb(uint64_t value) {
  do {
    uint8_t byte = value & 0x7f;
    value >>= 7;

    if (value != 0) byte |= 0x80;
    write<uint8_t>(byte);
  } while (value != 0);
}

void EHFrameWriter::write_sleb(int64_t value) {
  bool more;
  do {
    uint8_t byte = value & 0x7f;
    value >>= 7;

    more = !(value == 0  && (byte & 0x40) == 0) &&
           !(value == -1 && (byte & 0x40) != 0);

    if (more) byte |= 0x80;
    write<uint8_t>(byte);
  } while (more);
}

///////////////////////////////////////////////////////////////////////////////

EHFrameHandle EHFrameWriter::register_and_release() {
  auto& vec = *m_buf;
  auto const fde = m_fde;

  if (fde != kInvalidFDE) __register_frame(&vec[fde]);

  return std::shared_ptr<std::vector<uint8_t>>(
    m_buf.release(),
    [fde] (std::vector<uint8_t>* p) {
      SCOPE_EXIT { delete p; };
      if (fde != kInvalidFDE) __deregister_frame(&((*p)[fde]));
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

void EHFrameWriter::begin_cie(uint8_t rip, const void* personality) {
  assertx(m_buf != nullptr && m_buf->size() == 0);

  write<uint32_t>(0); // length; to be rewritten later
  write<uint32_t>(0); // CIE_id
  write<uint8_t>(1);  // version

  // Null-terminated augmentation string.
  write<char>('z');
  write<char>('R');
  if (personality) write<char>('P');
  write<char>('\0');

  // Code and data alignment factors.
  write_uleb(1);
  write_sleb(-8);

  // Return address register.  (One byte; this appears to be undocumented.)
  write<uint8_t>(rip);

  // Augmentation data length.  We compute this ahead-of-time because it has to
  // be encoded as a ULEB128, which is variable size.
  auto const ad_len = personality
    ? sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uintptr_t)
    : sizeof(uint8_t);
  write_uleb(ad_len);

  DEBUG_ONLY auto const ad_start = m_buf->size();

  // FDE address format.
  write<uint8_t>(DW_EH_PE_absptr);

  // Personality routine address format and address.
  if (personality != nullptr) {
    write<uint8_t>(DW_EH_PE_absptr);
    write<uintptr_t>(uintptr_t(personality));
  }
  assertx(ad_len == m_buf->size() - ad_start);
}

void EHFrameWriter::end_cie() {
  assertx(m_buf != nullptr && m_buf->size() != 0);
  assertx(m_fde == kInvalidFDE);
  auto& vec = *m_buf;

  // Patch the length field.  Note that it doesn't count the space for itself.
  auto vp = reinterpret_cast<uint32_t*>(&vec[0]);
  *vp = vec.size() - sizeof(uint32_t);
}

void EHFrameWriter::begin_fde(CodeAddress start, const uint8_t* cie) {
  assertx(m_buf != nullptr);
  assertx(m_fde == kInvalidFDE);
  auto& vec = *m_buf;

  m_fde = vec.size();

  // Assume the CIE was created at the start of the buffer if one was not
  // provided.
  if (cie == nullptr) {
    assertx(vec.size() != 0);
    cie = &vec[0];
  }

  // Reserve space for the FDE length.
  write<uint32_t>(0);

  // Negative offset to the CIE, relative to this field.
  //
  // TODO(#9732887): Figure out the appropriate type.  The documentation says
  // this should be unsigned, but libgcc uses a signed word.  Additionally,
  // nothing makes it clear whether it's required for all the FDE's of a CIE to
  // follow it contiguously (or even follow it at all).
  write<int32_t>(int32_t(&vec[vec.size()] - cie));

  // 8-byte pointer and 8-byte size describing the code range for this FDE.
  write<CodeAddress>(start);
  write<size_t>(0); // patched later

  // Augmentation data length.
  write_uleb(0);
}

void EHFrameWriter::end_fde(size_t size) {
  assertx(m_buf != nullptr && m_buf->size() != 0);
  assertx(m_fde != kInvalidFDE && m_fde < m_buf->size());
  auto& vec = *m_buf;

  // Patch the length field.  Note that it doesn't count the space for itself.
  auto vp_len = reinterpret_cast<uint32_t*>(&vec[m_fde]);
  *vp_len = vec.size() - m_fde - sizeof(uint32_t);

  // Patch the PC range field.
  auto const idx = m_fde
    + sizeof(uint32_t)      // FDE length
    + sizeof(int32_t)       // CIE_pointer
    + sizeof(CodeAddress);  // PC start
  auto vp_range = reinterpret_cast<size_t*>(&vec[idx]);
  *vp_range = size;
}

void EHFrameWriter::null_fde() {
  write<uint32_t>(0);
}

///////////////////////////////////////////////////////////////////////////////

void EHFrameWriter::def_cfa(uint8_t reg, uint64_t off) {
  write<uint8_t>(DW_CFA_def_cfa);
  write_uleb(reg);
  write_uleb(off);
}

void EHFrameWriter::same_value(uint8_t reg) {
  write<uint8_t>(DW_CFA_same_value);
  write_uleb(reg);
}

void EHFrameWriter::offset_extended_sf(uint8_t reg, int64_t off) {
  write<uint8_t>(DW_CFA_offset_extended_sf);
  write_uleb(reg);
  write_sleb(off);
}

///////////////////////////////////////////////////////////////////////////////

}
