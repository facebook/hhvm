#include "hphp/runtime/ext/icu/CodePointBreakIterator.h"

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
UOBJECT_DEFINE_RTTI_IMPLEMENTATION(CodePointBreakIterator);

typedef union {
  long    t1;
  double  t2;
  void   *t3;
} UAlignedMemory;

inline ptrdiff_t pointerMaskLSB(void *ptr, ptrdiff_t mask) {
  return (ptrdiff_t)ptr & mask;
}
inline size_t alignmentOffset(void *ptr) {
  return pointerMaskLSB(ptr, sizeof(UAlignedMemory) - 1);
}
inline size_t alignmentOffsetUp(void *ptr) {
  return sizeof(UAlignedMemory) - alignmentOffset(ptr);
}

CodePointBreakIterator*
CodePointBreakIterator::createBufferClone(void *stackBuffer,
                                          int32_t &bufferSize,
                                          UErrorCode &status) {
  if (U_FAILURE(status)) {
    return nullptr;
  }

  if (bufferSize <= 0) {
    bufferSize = sizeof(CodePointBreakIterator) + alignmentOffsetUp(0);
    return nullptr;
  }

  char *buf = static_cast<char*>(stackBuffer);
  uint32_t s = bufferSize;

  if (!stackBuffer) {
     s = 0;
  }

  if (alignmentOffset(stackBuffer) != 0) {
    uint32_t offsetUp = (uint32_t)alignmentOffsetUp(buf);
    s -= offsetUp;
    buf += offsetUp;
  }

  if (s < sizeof(CodePointBreakIterator)) {
    CodePointBreakIterator *clonedBI = new CodePointBreakIterator(*this);
    if (!clonedBI) {
      status = U_MEMORY_ALLOCATION_ERROR;
    } else {
      status = U_SAFECLONE_ALLOCATED_WARNING;
    }

    return clonedBI;
  }

  return new(buf) CodePointBreakIterator(*this);
}

#if U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM >= 409
CodePointBreakIterator&
CodePointBreakIterator::refreshInputText(UText *input,
           UErrorCode &status) {
  if (U_FAILURE(status)) {
    return *this;
  }
  if (!input) {
    status = U_ILLEGAL_ARGUMENT_ERROR;
    return *this;
  }

  int64_t pos = utext_getNativeIndex(m_text);
  m_text = utext_clone(m_text, input, false, true, &status);
  if (U_FAILURE(status)) {
    return *this;
  }

  utext_setNativeIndex(m_text, pos);
  if (utext_getNativeIndex(m_text) != pos) {
    status = U_ILLEGAL_ARGUMENT_ERROR;
  }

  return *this;
}
#endif

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
