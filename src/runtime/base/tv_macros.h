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
    (to)->m_data.num = (fr)->m_data.ptv->m_data.num; \
    (to)->_count = 0; \
    (to)->m_type = (fr)->m_data.ptv->m_type; \
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
// NOTE: this helper will not change the value of to->_count
#define TV_DUP_VAR_NC(fr,to) { \
  ASSERT((fr)->m_type == KindOfRef); \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = KindOfRef; \
  TV_INCREF(to); \
}

// Assumes 'fr' is live and 'to' is dead
// NOTE: this helper will not change the value of to->_count
#define TV_DUP_NC(fr,to) { \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = (fr)->m_type; \
  if (IS_REFCOUNTED_TYPE((to)->m_type)) { \
    TV_INCREF(to); \
  } \
}

// Assumes 'fr' is live and 'to' is dead
// Assumes 'fr->m_type != KindOfRef'
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP_CELL(fr, to) { \
  TV_DUP_CELL_NC((fr), (to)) \
  (to)->_count = 0; \
}

// Assumes 'fr' is live and 'to' is dead
// Assumes 'fr->m_type == KindOfRef'
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP_VAR(fr, to) { \
  ASSERT((fr)->m_type == KindOfRef); \
  TV_DUP_VAR_NC((fr), (to)) \
  (to)->_count = 0; \
}

// Assumes 'fr' is live and 'to' is dead
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP(fr,to) { \
  TV_DUP_NC((fr), (to)) \
  (to)->_count = 0; \
}

// Assumes 'fr' is live and 'to' is dead
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP_FLATTEN_VARS(fr, to, container) { \
  if (LIKELY(fr->m_type != KindOfRef)) { \
    TV_DUP_CELL_NC(fr, to); \
  } else if (fr->m_data.ptv->_count <= 1 && \
             ((container) == NULL || \
              fr->m_data.ptv->m_data.parr != container)) { \
    fr = fr->m_data.ptv; \
    TV_DUP_CELL_NC(fr, to); \
  } else { \
    TV_DUP_VAR_NC(fr, to); \
  } \
  to->_count = 0; \
}

// Assumes 'tv' is dead
// NOTE: this helper will initialize tv->_count to 0
#define TV_WRITE_NULL(tv) \
  (tv)->_count = 0; \
  (tv)->m_type = KindOfNull

// Assumes 'tv' is dead
// NOTE: this helper will initialize tv->_count to 0
#define TV_WRITE_UNINIT(tv) \
  (tv)->_count = 0; \
  (tv)->m_type = KindOfUninit

#endif
