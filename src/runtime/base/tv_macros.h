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

#ifndef __HPHP_TV_MACROS__
#define __HPHP_TV_MACROS__

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
#define TV_INCREF(tv) { \
  ASSERT(IS_REFCOUNTED_TYPE((tv)->m_type)); \
  (tv)->m_data.pstr->incRefCount(); \
}

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
// Assumes 'tv' is not shared (ie KindOfRef
//              or KindOfObject)
#define TV_INCREF_NOT_SHARED(tv) { \
  ASSERT((tv)->m_type == KindOfObject || \
         (tv)->m_type == KindOfRef); \
  (tv)->m_data.pobj->incRefCount(); \
}

// Assumes 'tv' is live
#define TV_UNBOX(tvptr) { \
  ASSERT((tvptr)->m_type == KindOfRef); \
  RefData* r = (tvptr)->m_data.pref; \
  TypedValue* innerCell = r->tv(); \
  (tvptr)->m_data.num = innerCell->m_data.num; \
  (tvptr)->m_type = innerCell->m_type; \
  if (IS_REFCOUNTED_TYPE((tvptr)->m_type)) { \
    TV_INCREF(tvptr); \
  } \
  tvDecRefRefInternal(r); \
}

// Assumes 'fr' is live and 'to' is dead
#define TV_READ_CELL(fr, to) { \
  if ((fr)->m_type != KindOfRef) { \
    memcpy((void*)(to), (void*)(fr), sizeof(TypedValue)); \
    if (IS_REFCOUNTED_TYPE((to)->m_type)) { \
      TV_INCREF(to); \
    } \
  } else { \
    TypedValue* fr2 = (fr)->m_data.pref->tv(); \
    (to)->m_data.num = fr2->m_data.num; \
    (to)->m_type = fr2->m_type; \
    if (IS_REFCOUNTED_TYPE((to)->m_type)) { \
      TV_INCREF(to); \
    } \
  } \
}

// Assumes 'fr' is live and 'to' is dead
// NOTE: this helper will not change the value of to->_count
#define TV_DUP_CELL_NC(fr, to) { \
  ASSERT((fr)->m_type != KindOfRef); \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = (fr)->m_type; \
  if (IS_REFCOUNTED_TYPE((to)->m_type)) { \
    TV_INCREF(to); \
  } \
}

// Assumes 'fr' is live and 'to' is dead
#define TV_DUP_VAR_NC(fr,to) { \
  ASSERT((fr)->m_type == KindOfRef); \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = KindOfRef; \
  TV_INCREF_NOT_SHARED(to); \
}

// Assumes 'fr' is live and 'to' is dead
#define TV_DUP_NC(fr,to) { \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = (fr)->m_type; \
  if (IS_REFCOUNTED_TYPE((to)->m_type)) { \
    TV_INCREF(to); \
  } \
}

// Assumes 'fr' is live and 'to' is dead
// Assumes 'fr->m_type != KindOfRef'
#define TV_DUP_CELL(fr, to) { \
  TV_DUP_CELL_NC((fr), (to)) \
}

// Assumes 'fr' is live and 'to' is dead
// Assumes 'fr->m_type == KindOfRef'
#define TV_DUP_VAR(fr, to) { \
  ASSERT((fr)->m_type == KindOfRef); \
  TV_DUP_VAR_NC((fr), (to)) \
}

// Assumes 'fr' is live and 'to' is dead
#define TV_DUP(fr,to) { \
  TV_DUP_NC((fr), (to)) \
}

// Assumes 'tv' is dead
#define TV_WRITE_NULL(tv) \
  (tv)->m_type = KindOfNull

// Assumes 'tv' is dead
#define TV_WRITE_UNINIT(tv) \
  (tv)->m_type = KindOfUninit

#endif
