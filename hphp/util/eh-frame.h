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

#ifndef HPHP_UTIL_EH_FRAME_H_
#define HPHP_UTIL_EH_FRAME_H_

#include "hphp/util/data-block.h"
#include "hphp/util/dwarf-reg.h"

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
#include "hphp/util/eh-frame-msvc.h"
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * A handle to a dynamically-generated .eh_frame section.
 *
 * Contains an exclusive pointer to a buffer, as well as routines for
 * inspecting its contents.  When destructed, it deregisters the .eh_frame
 * sections and frees the buffer.
 */
struct EHFrameDesc {
  ~EHFrameDesc();

  EHFrameDesc() {}
  EHFrameDesc(EHFrameDesc&&) = default;

  /*
   * Shared component (the "header") of CIEs and FDEs.
   */
  struct Entry {
    const uint8_t* get() const;
    uint32_t length() const;
    uint32_t cie_off() const;

    Entry(const std::vector<uint8_t>& vec, size_t idx)
      : m_vec(vec)
      , m_idx(idx)
    {}
    const std::vector<uint8_t>& m_vec;
    size_t m_idx;
  };

  /*
   * Helper structs for reading CIE and FDE data.
   */
  struct CIE : Entry {
    private: using Entry::Entry;
  };
  struct FDE : Entry {
    CodeAddress start() const;
    size_t range() const;

    private: using Entry::Entry;
  };

  CIE cie() const;

  template<class F>
  void for_each_fde(F f) const;

private:
  friend struct EHFrameWriter;
  std::unique_ptr<std::vector<uint8_t>> m_buf;
};

/*
 * API for dynamically creating and registering new .eh_frame sections.
 *
 * An EHFrameWriter instance can be given (or asked to create) an exclusive
 * pointer to a buffer.  It permits writing exactly one CIE, contiguously
 * followed by any number of FDES, before registering any FDEs written and
 * returning an EHFrameDesc---which, when destructed, deregisters and frees its
 * contents.
 *
 * In a pinch, EHFrameWriter could also just be used to write DWARF call frame
 * instructions to a buffer, without the .eh_frame specifics, though there are
 * currently assertions preventing that.
 */
struct EHFrameWriter {
  /*
   * Constructors.
   *
   * Either allocate a fresh buffer, or take exclusive ownership of the
   * provided `buf'.
   */
  EHFrameWriter()
    : m_buf(std::make_unique<std::vector<uint8_t>>())
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
  EHFrameDesc register_and_release();

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
   *  CIE pointer:    offset of `&m_buf[m_cie.idx]'
   *  initial PC:     `start'
   *  address range:  `size'
   *  augmentation:   0
   *
   * Any call frame instructions should be written between the calls to
   * begin_fde() and end_fde().
   */
  void begin_fde(CodeAddress start);
  void end_fde(size_t size);

  /*
   * Write an FDE with zero length.
   *
   * This is required at the end of an .eh_frame section, to indicate that
   * there are no more FDEs sharing their CIE.  (This requirement does not
   * appear to be documented, but failing to zero-terminate causes a segfault
   * in classify_object_over_fdes() in unwind-dw2-fde.c.
   */
  void null_fde();

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Write a DWARF call frame instruction to the buffer.
   *
   * These all emit DW_CFA opcodes with the appropriate arguments.  For
   * documentation, see Chapter 6.4.2: Call Frame Instructions in
   * http://dwarfstd.org/doc/DWARF4.pdf
   */

  /*
   * Row creation instruction.
   *
   * Emit the appropriate advance_loc* instruction, or set_loc if we are
   * advancing more than 2^32 bytes.
   */
  void advance_loc_to(CodeAddress addr);

  /*
   * CFA definition instructions.
   */
  void def_cfa(uint8_t reg, uint64_t off);
  void def_cfa_register(uint8_t reg);
  void def_cfa_offset(uint64_t off);
  /*
   * Adjust the tracked CFA offset, and emit a def_cfa_offset for the new one.
   */
  void advance_cfa_offset(int64_t off);

  /*
   * Register rule instructions.
   */
  void same_value(uint8_t reg);
  void offset(uint8_t reg, uint64_t off);
  void offset_extended_sf(uint8_t reg, int64_t off);
  void restore(uint8_t reg);
  /*
   * Begin a DWARF expression which either:
   * - expression():     computes the memory location which contains the value
   *                     of `reg'; or
   * - val_expression(): computes the value of `reg'.
   *
   * The operations comprising the expression should be written between the
   * calls to begin_{,val_}expression() and end_expression() (which ends either
   * kind of expression).
   *
   * At present, we always use DW_FORM_block1, and attempting to write an
   * expression whose operations contain more than (2^8 - 1) bytes will fail an
   * assertion.
   */
  void begin_expression(uint8_t reg);
  void begin_val_expression(uint8_t reg);
  void end_expression();

  /*
   * Padding instruction.
   */
  void nop();

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Write a DWARF operation to the buffer.
   *
   * These all emit DW_OP opcodes with the appropriate arguments.  For
   * documentation, see Chapter 2.5: DWARF Expressions
   * http://dwarfstd.org/doc/DWARF4.pdf
   */

  /*
   * Literal encodings.
   */
  void op_consts(int64_t cns);

  /*
   * Register based addressing.
   */
  void op_breg(uint8_t reg, int64_t off);

  /*
   * Stack operations.
   */
  void op_dup();
  void op_drop();
  void op_swap();
  void op_deref();

  /*
   * Arithmetic and logical operations.
   */
  void op_abs();
  void op_and();
  void op_div();
  void op_minus();
  void op_mod();
  void op_mul();
  void op_neg();
  void op_not();
  void op_or();
  void op_plus();


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

  void dw_cfa(uint8_t);
  void dw_op(uint8_t);
  void write_uleb(uint64_t);
  void write_sleb(int64_t);

  void begin_expr_impl(uint8_t op, uint8_t reg);

  static constexpr size_t kInvalidIdx = -1ull;

  /////////////////////////////////////////////////////////////////////////////

private:
  // The managed buffer.
  std::unique_ptr<std::vector<uint8_t>> m_buf{nullptr};

  // Metadata for the emitted CIE.
  struct {
    // Index of the CIE in m_buf.
    size_t idx{kInvalidIdx};
    // The CIE's data_alignment_factor.
    int64_t data_align{-8};
    // CFA offset at the end of the CIE's call frame instructions.
    uint64_t offset{0};
  } m_cie;

  // Index in m_buf of the FDE currently being written.
  size_t m_fde{kInvalidIdx};
  // Current CFI table row address.
  CodeAddress m_loc{nullptr};
  // Current CFA offset.
  uint64_t m_off{0};

  // Index in m_buf of the expression currently being written.
  size_t m_expr{kInvalidIdx};
};

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/util/eh-frame-inl.h"

#endif
