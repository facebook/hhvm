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

#include <runtime/base/fiber_reference_map.h>
#include <runtime/base/fiber_async_func.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void FiberReferenceMap::insert(ObjectData *src, ObjectData *copy) {
  insert((void*)src, (void*)copy);
}

void FiberReferenceMap::insert(Variant *src, Variant *copy) {
  insert((void*)src, (void*)copy);

  // We need strongly bound Variants to have ref count > 1, so we will
  // maintain "copy" pointer able to map back to original Variant*.
  copy->incRefCount();
  Variant tmp(copy);
  m_refVariants.append(ref(tmp));
}

void FiberReferenceMap::insert(void *src, void *copy) {
  ASSERT(lookup(src) == NULL);
  ASSERT(copy == NULL || reverseLookup(copy) == NULL);
  m_forward_references[src] = copy;
  if (copy) {
    m_reverse_references[copy] = src;
  }
}

void *FiberReferenceMap::lookup(void *src) {
  PointerMap::iterator iter = m_forward_references.find(src);
  if (iter != m_forward_references.end()) {
    return iter->second;
  }
  return NULL;
}

void *FiberReferenceMap::reverseLookup(void *copy) {
  PointerMap::iterator iter = m_reverse_references.find(copy);
  if (iter != m_reverse_references.end()) {
    return iter->second;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void FiberReferenceMap::marshal(String &dest, String &src) {
  dest = src.fiberCopy();
}

void FiberReferenceMap::marshal(Array &dest, Array &src) {
  dest = src.fiberMarshal(*this);
}

void FiberReferenceMap::marshal(Object &dest, Object &src) {
  dest = src.fiberMarshal(*this);
}

void FiberReferenceMap::marshal(Variant &dest, Variant &src) {
  dest = src.fiberMarshal(*this);
}

void FiberReferenceMap::unmarshal(String &dest, String &src, char strategy) {
  if (strategy != FiberAsyncFunc::GlobalStateIgnore) {
    dest = src.fiberCopy();
  }
}

void FiberReferenceMap::unmarshal(Array &dest, Array &src, char strategy) {
  switch (strategy) {
    case FiberAsyncFunc::GlobalStateIgnore:
      // do nothing
      break;
    case FiberAsyncFunc::GlobalStateOverwrite:
      dest = src.fiberUnmarshal(*this);
      break;
    case FiberAsyncFunc::GlobalStateSkip:
      for (ArrayIter iter(src); iter; ++iter) {
        Variant key = iter.first();
        if (!dest.exists(key)) {
          dest.set(key.fiberUnmarshal(*this),
                   ref(iter.secondRef().fiberUnmarshal(*this)));
        }
      }
      break;
    default:
      raise_error("unknown strategy: %d", (int)strategy);
      break;
  }
}

void FiberReferenceMap::unmarshal(Object &dest, Object &src, char strategy) {
  if (strategy != FiberAsyncFunc::GlobalStateIgnore) {
    dest = src.fiberUnmarshal(*this);
  }
}

void FiberReferenceMap::unmarshal(Variant &dest, CVarRef src, char strategy) {
  if (dest.isArray() && src.isArray()) {
    switch (strategy) {
      case FiberAsyncFunc::GlobalStateIgnore:
        // do nothing
        break;
      case FiberAsyncFunc::GlobalStateOverwrite:
        dest = src.fiberUnmarshal(*this);
        break;
      case FiberAsyncFunc::GlobalStateSkip: {
        Array arr = dest.toArray();
        for (ArrayIter iter(src); iter; ++iter) {
          Variant key = iter.first();
          if (!arr.exists(key)) {
            dest.set(key.fiberUnmarshal(*this),
                     ref(iter.secondRef().fiberUnmarshal(*this)));
          }
        }
        break;
      }
      default:
        raise_error("unknown strategy: %d", (int)strategy);
        break;
    }
  } else if (strategy != FiberAsyncFunc::GlobalStateIgnore) {
    dest = src.fiberUnmarshal(*this);
  }
}

void FiberReferenceMap::unmarshalDynamicGlobals
(Array &dest, Array &src, char default_strategy,
 const hphp_string_map<char> &additional_strategies) {
  for (ArrayIter iter(src); iter; ++iter) {
    String key = iter.first().toString();
    CVarRef val = iter.secondRef();
    String fullKey = String("gv_") + key;

    FiberAsyncFunc::Strategy strategy =
      (FiberAsyncFunc::Strategy)default_strategy;
    hphp_string_map<char>::const_iterator it =
      additional_strategies.find(fullKey.data());
    if (it != additional_strategies.end()) {
      strategy = (FiberAsyncFunc::Strategy)it->second;
    }

    Variant &dval = dest.lvalAt(key.fiberCopy());
    unmarshal(dval, val, strategy);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
