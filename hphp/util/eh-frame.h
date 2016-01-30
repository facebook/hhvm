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

#ifndef HPHP_UTIL_EH_FRAME_H_
#define HPHP_UTIL_EH_FRAME_H_

#include "hphp/util/data-block.h"

#include <boost/mpl/identity.hpp>
#include <folly/Memory.h>

#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#ifndef _MSC_VER
#include <dwarf.h>
#include <libdwarf.h>
#else
#include "hphp/runtime/vm/debug/eh-frame-msvc.h"
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace dw_reg {

enum X64Reg : uint8_t {
  RAX, RDX, RCX, RBX, RSI, RDI, RBP, RSP,
  R8,  R9,  R10, R11, R12, R13, R14, R15, RIP
};

};

///////////////////////////////////////////////////////////////////////////////

using EHFrameHandle = std::shared_ptr<const std::vector<uint8_t>>;

/*
 * API for dynamically creating and registering new .eh_frame sections.
 *
 * An EHFrameWriter instance can be given (or asked to create) an exclusive
 * pointer to a buffer.  It permits writing up to one CIE and up to one FDE,
 * before registering the FDE (if one was written) and returning a shared_ptr
 * to the buffer---which deregisters itself and frees its memory when all
 * references to it are released.
 *
 * In a pinch, EHFrameWriter can also just be used to write DWARF call frame
 * instructions to a buffer, without the .eh_frame specifics.
 */
struct EHFrameWriter {
  /*
   * Constructors.
   *
   * Either allocate a fresh buffer, or take exclusive ownership of the
   * provided `buf'.
   */
  EHFrameWriter()
    : m_buf(folly::make_unique<std::vector<uint8_t>>())
  {}
  explicit EHFrameWriter(std::unique_ptr<std::vector<uint8_t>>&& buf)
    : m_buf(std::move(buf))
  {}

  /*
   * Register the FDE written to the buffer (if one was written), then release
   * the buffer.
   *
   * When all references to the returned value are lost, the buffer will delete
   * itself and deregister its FDE (if it exists).
   */
  EHFrameHandle register_and_release();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Write a CIE with the following fields:
   *
   *  length:       automatic
   *  CIE id:       0
   *  version:      1
   *  augment str:  personality ? "zPR" : "zR"
   *  code align:   1
   *  data align:   -8
   *  return reg:   rip
   *  augmentation: all pointer encodings DW_EH_PE_absptr
   *
   * Any initial call frame instructions should be written between the calls to
   * begin_cie() and end_cie().
   *
   * @see: https://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-PDA/LSB-PDA/ehframechpt.html
   *       https://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-PDA/LSB-PDA.junk/dwarfext.html
   */
  void begin_cie(uint8_t rip, const void* personality = nullptr);
  void end_cie();

  /*
   * Write an FDE with the following fields:
   *
   *  length:         automatic
   *  CIE pointer:    offset of `cie ? cie : &m_buf[0]'
   *  initial PC:     `start'
   *  address range:  `size'
   *  augmentation:   0
   *
   * Any call frame instructions should be written between the calls to
   * begin_fde() and end_fde().
   */
  void begin_fde(CodeAddress start, const uint8_t* cie = nullptr);
  void end_fde(size_t size);

  /*
   * Write an FDE with zero length.
   */
  void null_fde();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Write a DWARF call frame instruction to the buffer.
   *
   * These all emit DW_CFA opcodes with the appropriate arguments.  For
   * documentation, see: http://dwarfstd.org/doc/DWARF4.pdf
   */
  void def_cfa(uint8_t reg, uint64_t off);

  void same_value(uint8_t reg);
  void offset_extended_sf(uint8_t reg, int64_t off);

  /////////////////////////////////////////////////////////////////////////////

private:
  template<class T>
  void write(// Prevent template argument deduction:
             typename boost::mpl::identity<T>::type t) {
    auto& v = *m_buf;
    auto const idx = v.size();
    v.resize(idx + sizeof(T));
    auto caddr = &v[idx];
    std::memcpy(caddr, &t, sizeof t);
  }

  void write_uleb(uint64_t);
  void write_sleb(int64_t);

  static constexpr size_t kInvalidFDE = -1ull;

  /////////////////////////////////////////////////////////////////////////////

private:
  // The managed buffer.
  std::unique_ptr<std::vector<uint8_t>> m_buf{nullptr};
  // Index of the FDE in m_buf; invalid if no FDE was written.
  size_t m_fde{kInvalidFDE};
};

///////////////////////////////////////////////////////////////////////////////

}

#endif
