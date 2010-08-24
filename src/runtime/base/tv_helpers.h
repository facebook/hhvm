/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'tv_helpers.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_TV_HELPERS_H__
#define __HPHP_TV_HELPERS_H__

#include <runtime/base/types.h>

// We want to use the TypedValue macros inside this header file, but we don't
// want to pollute the environment of files that include this header file.
// Thus, we record whether they were already defined here. That way, we know
// whether we need to undefine the macros at the end of this header file.
#ifndef __HPHP_TV_MACROS__
#include <runtime/base/tv_macros.h>
#define __HPHP_TV_HELPERS_H_SHOULD_UNDEF_TV__
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Assumes 'tv' is live
//
// Assumes 'tv->m_type != KindOfVariant'
#ifdef FAST_REFCOUNT_FOR_VARIANT
inline void tvDecRefCell(TypedValue* tv) {
  if (tv->m_data.pvar->decRefCount() == 0) {
    if (tv->m_type < KindOfArray) {
      ASSERT(tv->m_type == KindOfString);
      tv->m_data.pstr->release();
    } else {
      if (tv->m_type != KindOfArray) {
        ASSERT(tv->m_type == KindOfObject);
        tv->m_data.pobj->release();
      } else {
        tv->m_data.parr->release();
      }
    }
  }
}
#else
inline void tvDecRefCell(TypedValue* tv) {
  if (tv->m_type < KindOfArray) {
    ASSERT(tv->m_type == KindOfString);
    if (tv->m_data.pstr->decRefCount() == 0) {
      tv->m_data.pstr->release();
    }
  } else {
    if (tv->m_type != KindOfArray) {
      ASSERT(tv->m_type == KindOfObject);
      if (tv->m_data.pobj->decRefCount() == 0) {
        tv->m_data.pobj->release();
      }
    } else {
      if (tv->m_data.parr->decRefCount() == 0) {
        tv->m_data.parr->release();
      }
    }
  }
}
#endif

// Assumes 'tv' is live
//
// Assumes 'tv->m_type != KindOfVariant && tv->_count > 0'
inline void tvDecRefVarInternal(TypedValue* tv) {
  ASSERT(tv->m_type != KindOfVariant);
  ASSERT(tv->_count > 0);
  if (((Variant*)tv)->decRefCount() == 0) {
    ((Variant*)tv)->release();
  }
}

// Assumes 'tv' is live
//
// Assumes 'tv->m_type == KindOfVariant'
inline void tvDecRefVar(TypedValue* tv) {
  ASSERT(tv->m_type == KindOfVariant);
  tvDecRefVarInternal(tv->m_data.ptv);
}

// Assumes 'tv' is live
#ifdef FAST_REFCOUNT_FOR_VARIANT
inline void tvDecRef(TypedValue* tv) {
  if (tv->m_data.pvar->decRefCount() == 0) {
    if (tv->m_type == KindOfString) {
      tv->m_data.pstr->release();
    } else {
      if (tv->m_type < KindOfObject) {
        ASSERT(tv->m_type == KindOfArray);
        tv->m_data.parr->release();
      } else {
        if (tv->m_type != KindOfObject) {
          ASSERT(tv->m_type == KindOfVariant);
          tv->m_data.pvar->release();
        } else {
          tv->m_data.pobj->release();
        }
      }
    }
  }
}
#else
inline void tvDecRef(TypedValue* tv) {
  if (tv->m_type == KindOfString) {
    if (tv->m_data.pstr->decRefCount() == 0) {
      tv->m_data.pstr->release();
    }
  } else {
    if (tv->m_type < KindOfObject) {
      ASSERT(tv->m_type == KindOfArray);
      if (tv->m_data.parr->decRefCount() == 0) {
        tv->m_data.parr->release();
      }
    } else {
      if (tv->m_type != KindOfObject) {
        ASSERT(tv->m_type == KindOfVariant);
        if (tv->m_data.pvar->decRefCount() == 0) {
          tv->m_data.pvar->release();
        }
      } else {
        if (tv->m_data.pobj->decRefCount() == 0) {
          tv->m_data.pobj->release();
        }
      }
    }
  }
}
#endif

// Assumes 'tv' is live
//
// Assumes 'tv.m_type != KindOfVariant'
inline void tvBox(TypedValue* tv) {
  ASSERT(tv->m_type != KindOfVariant);
  ASSERT(tv->_count == 0);
  TypedValue* innerCell = (TypedValue*)(NEW(Variant)());
  innerCell->m_data.num = (tv)->m_data.num;
  innerCell->_count = 1;
  innerCell->m_type = (tv)->m_type;
  tv->m_data.ptv = innerCell;
  tv->m_type = KindOfVariant;
}

// Assumes 'tv' is live
//
// Assumes 'tv.m_type == KindOfVariant'
inline void tvUnbox(TypedValue* tv) {
  TV_UNBOX(tv);
}

// Assumes 'fr' is live and 'to' is dead
inline void tvReadCell(const TypedValue* fr, TypedValue* to) {
  TV_READ_CELL(fr, to);
}

// Assumes 'fr' is live and 'to' is dead
inline void tvDupCell(const TypedValue* fr, TypedValue* to) {
  TV_DUP_CELL(fr, to);
}

// Assumes 'to' is dead and 'fr' is live
//
// Assumes 'fr->m_type == KindOfVariant'
inline void tvDupVar(const TypedValue* fr, TypedValue* to) {
  TV_DUP_VAR(fr,to);
}

// Assumes 'to' is dead and 'fr' is live
inline void tvDup(const TypedValue* fr, TypedValue* to) {
  TV_DUP(fr,to);
}

// Assumes 'tv' is dead
inline void tvWriteNull(TypedValue* tv) {
  TV_WRITE_NULL(tv);
}

// Assumes 'tv' is dead
inline void tvWriteUninit(TypedValue* tv) {
  TV_WRITE_UNINIT(tv);
}

// Assumes 'to' and 'fr' are live
//
// Assumes that 'fr->m_type != KindOfVariant'
//
// If 'to->m_type == KindOfVariant', this will
// perform the set operation on the inner cell
// (to->m_data.ptv)
inline void tvSet(TypedValue * fr, TypedValue * to) {
  ASSERT(fr->m_type != KindOfVariant);
  if (to->m_type == KindOfVariant) {
    to = to->m_data.ptv;
  }
  if (to->m_type > KindOfStaticString) {
    tvDecRefCell(to);
  }
  to->m_data.num = fr->m_data.num;
  to->m_type = fr->m_type;
  // If we just copied a complex type, we need to bump
  // up the refcount
  if (fr->m_type > KindOfStaticString) {
    TV_INCREF(fr);
  }
}

/**
 * tvAsVariant and tvAsCVarRef serve as escape hatches that allows us to call
 * into the Variant machinery. Ideally we will use these as little as possible
 * in the long term.
 */

// Assumes 'tv' is live
inline Variant& tvAsVariant(TypedValue* tv) {
  return *(Variant*)(tv);
}

// Assumes 'tv' is live
inline const Variant& tvAsCVarRef(const TypedValue* tv) {
  return *(const Variant*)(tv);
}

// Assumes 'tv' is live
inline Variant& tvCellAsVariant(TypedValue* tv) {
  ASSERT(tv->m_type != KindOfVariant);
  return *(Variant*)(tv);
}

// Assumes 'tv' is live
inline const Variant& tvCellAsCVarRef(const TypedValue* tv) {
  ASSERT(tv->m_type != KindOfVariant);
  return *(const Variant*)(tv);
}

// Assumes 'tv' is live
inline Variant& tvVarAsVariant(TypedValue* tv) {
  ASSERT(tv->m_type == KindOfVariant);
  return *(Variant*)(tv);
}

// Assumes 'tv' is live
inline const Variant& tvVarAsCVarRef(const TypedValue* tv) {
  ASSERT(tv->m_type == KindOfVariant);
  return *(const Variant*)(tv);
}

///////////////////////////////////////////////////////////////////////////////
}

// Undefine the TypedValue macros if appropriate
#ifdef __HPHP_TV_HELPERS_H_SHOULD_UNDEF_TV__
#undef __HPHP_TV_HELPERS_H_SHOULD_UNDEF_TV__
#include <runtime/base/undef_tv_macros.h>
#endif

#endif // __HPHP_HPHPVALUE_H__
