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

#ifdef __HPHP_TV_MACROS__
#error Including 'tv_macros.h' multiple times is prohibited.
#endif

#define __HPHP_TV_MACROS__

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
#ifdef FAST_REFCOUNT_FOR_VARIANT
#define TV_INCREF(tv) { \
  ASSERT(IS_REFCOUNTED_TYPE((tv)->m_type)); \
  (tv)->m_data.pstr->incRefCount(); \
}
#else
#define TV_INCREF(tv) { \
  ASSERT(IS_REFCOUNTED_TYPE((tv)->m_type)); \
  if ((tv)->m_type == KindOfString) { \
    (tv)->m_data.pstr->incRefCount(); \
  } else { \
    if ((tv)->m_type < KindOfObject) { \
      ASSERT((tv)->m_type == KindOfArray); \
      (tv)->m_data.parr->incRefCount(); \
    } else { \
      if ((tv)->m_type != KindOfObject) { \
        ASSERT((tv)->m_type == KindOfVariant); \
        (tv)->m_data.pvar->incRefCount(); \
      } else { \
        (tv)->m_data.pobj->incRefCount(); \
      } \
    } \
  } \
}
#endif

// Assumes 'tv' is live
#define TV_UNBOX(tv) { \
  ASSERT((tv)->m_type == KindOfVariant); \
  TypedValue* innerCell = (tv)->m_data.ptv; \
  (tv)->m_data.num = innerCell->m_data.num; \
  (tv)->m_type = innerCell->m_type; \
  if (IS_REFCOUNTED_TYPE((tv)->m_type)) { \
    TV_INCREF(tv); \
  } \
  ASSERT(innerCell->_count > 0); \
  tvDecRefVarInternal(innerCell); \
}

// Assumes 'fr' is live and 'to' is dead
#define TV_READ_CELL(fr, to) { \
  if ((fr)->m_type != KindOfVariant) { \
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
  ASSERT((fr)->m_type != KindOfVariant); \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = (fr)->m_type; \
  if (IS_REFCOUNTED_TYPE((to)->m_type)) { \
    TV_INCREF(to); \
  } \
}

// Assumes 'fr' is live and 'to' is dead
// NOTE: this helper will not change the value of to->_count
#define TV_DUP_VAR_NC(fr,to) { \
  ASSERT((fr)->m_type == KindOfVariant); \
  (to)->m_data.num = (fr)->m_data.num; \
  (to)->m_type = KindOfVariant; \
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
// Assumes 'fr->m_type != KindOfVariant'
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP_CELL(fr, to) { \
  TV_DUP_CELL_NC((fr), (to)) \
  (to)->_count = 0; \
}

// Assumes 'fr' is live and 'to' is dead
// Assumes 'fr->m_type == KindOfVariant'
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP_VAR(fr, to) { \
  TV_DUP_VAR_NC((fr), (to)) \
  (to)->_count = 0; \
}

// Assumes 'fr' is live and 'to' is dead
// NOTE: this helper will initialize to->_count to 0
#define TV_DUP(fr,to) { \
  TV_DUP_NC((fr), (to)) \
  (to)->_count = 0; \
}

// Assumes 'tv' is dead
// NOTE: this helper will initialize tv->_count to 0
#define TV_WRITE_NULL(tv) \
  (tv)->m_data.num = 0; \
  (tv)->_count = 0; \
  (tv)->m_type = KindOfNull

// Assumes 'tv' is dead
// NOTE: this helper will initialize tv->_count to 0
#define TV_WRITE_UNINIT(tv) \
  (tv)->m_data.num = 0; \
  (tv)->_count = 0; \
  (tv)->m_type = KindOfUninit

