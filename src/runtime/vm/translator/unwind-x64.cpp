/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/vm/translator/unwind-x64.h"
#include <libunwind.h>
#include <unwind.h>
#include <vector>
#include <memory>
#include <boost/mpl/identity.hpp>

#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/translator/runtime-type.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/vm/stats.h"
#include "runtime/vm/runtime.h"

// libgcc exports this for registering eh information for
// dynamically-loaded objects.  The pointer is to data in the format
// you find in a .eh_frame section.
extern "C" void __register_frame(void*);
extern "C" void __deregister_frame(void*);

namespace HPHP { namespace VM { namespace Transl {

//////////////////////////////////////////////////////////////////////

const Trace::Module TRACEMOD = Trace::tunwind;

//////////////////////////////////////////////////////////////////////

namespace {

template<class T>
void append_vec(std::vector<char>& v,
                // Prevent template argument deduction:
                typename boost::mpl::identity<T>::type t) {
  size_t idx = v.size();
  v.resize(idx + sizeof(T));
  char* caddr = &v[idx];
  std::memcpy(caddr, &t, sizeof t);
}

void sync_regstate(_Unwind_Context* context) {
  ASSERT(tl_regState == REGSTATE_DIRTY);

  TRACE(1, "unwind: doing fixup sync\n");

  uintptr_t frameRbp = _Unwind_GetGR(context, Debug::RBP);
  uintptr_t frameRip = _Unwind_GetGR(context, Debug::RIP);

  /*
   * fixupWork expects to be looking at the first frame that is out of
   * the TC.  We have RBP/RIP for the TC frame that called out here,
   * so we make a fake ActRec here to give it what it expects.
   *
   * Note: this doesn't work for IndirectFixup situations.  However,
   * currently IndirectFixup is only used for destructors, which
   * aren't allowed to throw, so this is ok.
   */
  ActRec fakeAr;
  fakeAr.m_savedRbp = frameRbp;
  fakeAr.m_savedRip = frameRip;

  Stats::inc(Stats::TC_SyncUnwind);
  tx64->fixupWork(g_vmContext, &fakeAr);
  tl_regState = REGSTATE_CLEAN;
}

void sync_callee_saved(_Unwind_Context* context) {
  const CTCA frameIP = CTCA(_Unwind_GetGR(context, Debug::RIP));
  const UnwindRegInfo* const uInfo = tx64->getUnwindInfo(frameIP);
  if (!uInfo) return;

  ActRec* const frameAr = reinterpret_cast<ActRec*>(
    _Unwind_GetGR(context, Debug::RBP));

  TRACE(1, "unwind: cleaning callee dirty regs\n");
  for (int i = 0; i < UnwindRegInfo::kMaxCalleeSaved; ++i) {
    UnwindRegInfo::Data cri = uInfo->m_regs[i];
    if (!cri.dirty) break;

    TRACE(1, "unwind: unwind reg %s (val: %p)\n", cri.pretty().c_str(),
            (void*)_Unwind_GetGR(context, cri.reg));

    uintptr_t contents = _Unwind_GetGR(context, cri.reg);
    TypedValue* cell;
    if (cri.exStack) {
      /*
       * We don't keep information about how to restore the value of
       * rVmSp (rbx) generally in tc frames, however the only time
       * we'll need to sync dirty regs here is if we are the first tc
       * frame below a C++ frame.  In this case, rbx is properly
       * restored for us by the unwinder, so we can use it here.
       */
      static_assert(rVmSp == reg::rbx,
                    "unwind-x64.cpp expected rVmSp == rbx");
      TypedValue* frameSp = reinterpret_cast<TypedValue*>(
        _Unwind_GetGR(context, Debug::RBX));
      cell = frameSp - (cri.locOffset + 1);
    } else {
      cell = frame_local(frameAr, cri.locOffset);
    }

    cell->m_type = DataType(cri.dataType);
    cell->m_data.num = contents;
  }
}

_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context) {
  ASSERT(version == 1);

  /*
   * We don't do anything during the search phase---before attempting
   * cleanup, we want all deeper frames to have run their object
   * destructors (which can have side effects like setting
   * tl_regState) and spilled any values they may have been holding in
   * callee-saved regs.
   */
  if (actions & _UA_SEARCH_PHASE) {}

  /*
   * During the cleanup phase, we can either use a landing pad to
   * perform cleanup (with _Unwind_SetIP and _URC_INSTALL_CONTEXT), or
   * we can do it here.  We do it here currently so we can use unwind
   * APIs to read the callee-saved register values instead of having
   * to generate machine code that does it.
   */
  else if (actions & _UA_CLEANUP_PHASE) {
    if (tl_regState == REGSTATE_DIRTY) {
      sync_regstate(context);
    }
    sync_callee_saved(context);
  }

  return _URC_CONTINUE_UNWIND;
}

void deregister_unwind_region(std::vector<char>* p) {
  std::auto_ptr<std::vector<char> > del(p);
  __deregister_frame(&(*p)[0]);
}

}

//////////////////////////////////////////////////////////////////////

UnwindRegInfo::UnwindRegInfo() {
  clear();
}

std::string UnwindRegInfo::Data::pretty() const {
  std::ostringstream out;
  out << "(CRI r"
      << reg << " dt:" << dataType << ' '
      << (exStack ? "stack" : "local")
      << '@' << locOffset
      << ")";
  return out.str();
}

void UnwindRegInfo::clear() {
  memset(m_regs, 0, sizeof m_regs);
}

bool UnwindRegInfo::empty() const {
  return !m_regs[0].dirty;
}

void UnwindRegInfo::add(RegNumber reg,
                        DataType type,
                        Location loc) {
  ASSERT(type >= -128 && type < 128 &&
         "UnwindRegInfo has only 8 bits for DataType");
  ASSERT(loc.space == Location::Stack || loc.space == Location::Local);
  ASSERT(loc.offset >= std::numeric_limits<int16_t>::min() &&
         loc.offset <= std::numeric_limits<int16_t>::max() &&
         "UnwindRegInfo only has 16 bits for location offsets");

  Data ent;
  ent.dirty = true;
  ent.dataType = type;
  ent.exStack = loc.space == Location::Stack;
  ent.locOffset = loc.offset;

  /*
   * Important note: this is using asm-x64.h register numbers,
   * although they are not the same as what dwarf/unwind uses.
   *
   * They happen to be the same for the specific (callee-saved) regs
   * we care about right now, so this is ok.  See kCalleeSaved.
   */
  ent.reg = int(reg);

  for (int i = 0; i < kMaxCalleeSaved; ++i) {
    if (!m_regs[i].dirty) {
      m_regs[i] = ent;
      return;
    }
  }

  ASSERT(false && "Too many callee saved registers for UnwindRegInfo");
}

UnwindInfoHandle
register_unwind_region(unsigned char* startAddr, size_t size) {
  std::auto_ptr<std::vector<char> > bufferMem(new std::vector<char>);
  std::vector<char>& buffer = *bufferMem;

  {
    // This is a dwarf CIE header.  Looks the same as a fde except the
    // second field is zero.
    append_vec<uint32_t>(buffer, 0); // Room for length later
    append_vec<int32_t>(buffer, 0);  // CIE_id
    append_vec<uint8_t>(buffer, 1);  // version

    /*
     * Null-terminated "augmentation string" (defines what the rest of
     * this thing is going to have.
     *
     * TODO: probably we should have a 'z' field to indicate length.
     */
    append_vec<char>(buffer, 'P');
    append_vec<char>(buffer, '\0');

    // Code and data alignment.
    append_vec<uint8_t>(buffer, 1);
    append_vec<uint8_t>(buffer, 8); // Multiplies offsets below.

    // Return address column (in version 1, this is a single byte).
    append_vec<uint8_t>(buffer, Debug::RIP);

    // Pointer to the personality routine for the TC.
    append_vec<uint8_t>(buffer, DW_EH_PE_absptr);
    append_vec<uintptr_t>(buffer, uintptr_t(tc_unwind_personality));

    /*
     * Define a program for the CIE.  This explains to the unwinder
     * how to figure out where the frame pointer was, etc.
     *
     * Arguments to some of these are encoded in LEB128, so we have to
     * clear the high bit for the signed values.
     */
    // Previous FP (CFA) is at rbp + 16.
    append_vec<uint8_t>(buffer, DW_CFA_def_cfa);
    append_vec<uint8_t>(buffer, Debug::RBP);
    append_vec<uint8_t>(buffer, 16);
    // rip is at CFA - 1 * data_align.
    append_vec<uint8_t>(buffer, DW_CFA_offset_extended_sf);
    append_vec<uint8_t>(buffer, Debug::RIP);
    append_vec<uint8_t>(buffer, -1u & 0x7f);
    // rbp is at CFA - 2 * data_align.
    append_vec<uint8_t>(buffer, DW_CFA_offset_extended_sf);
    append_vec<uint8_t>(buffer, Debug::RBP);
    append_vec<uint8_t>(buffer, -2u & 0x7f);
    /*
     * Leave rsp unchanged.
     *
     * Note that some things in the translator do actually change rsp,
     * but we assume they cannot throw so this is ok.  If rVmSp ever
     * changes to use rsp this code must change.
     */
    append_vec<uint8_t>(buffer, DW_CFA_same_value);
    append_vec<uint8_t>(buffer, Debug::RSP);

    // Fixup the length field.  Note that it doesn't include the space
    // for itself.
    void* vp = &buffer[0];
    *static_cast<uint32_t*>(vp) = buffer.size() - sizeof(uint32_t);
  }
  const size_t fdeIdx = buffer.size();
  {
    // Reserve space for FDE length.
    append_vec<uint32_t>(buffer, 0);

    // Negative offset to the CIE for this FDE---the offset is
    // relative to this field.
    append_vec<int32_t>(buffer, int32_t(buffer.size()));

    // We're using the addressing mode DW_EH_PE_absptr, which means it
    // wants a 8 byte pointer and a 8 byte size indicating the region
    // this FDE applies to.
    append_vec<unsigned char*>(buffer, startAddr);
    append_vec<size_t>(buffer, size);

    // Fixup the length field for this FDE.  Again length doesn't
    // include the length field itself.
    void* vp = &buffer[fdeIdx];
    *static_cast<uint32_t*>(vp) = buffer.size() - fdeIdx - sizeof(uint32_t);
  }
  // Add one more zero'd length field---this indicates that there are
  // no more FDEs sharing this CIE.
  append_vec<uint32_t>(buffer, 0);

  __register_frame(&buffer[0]);

  return boost::shared_ptr<std::vector<char> >(
    bufferMem.release(),
    deregister_unwind_region
  );
}

//////////////////////////////////////////////////////////////////////

}}}
