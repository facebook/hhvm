/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_iterator.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_splfile.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJECT_ALLOCATION(DirectoryIterator)
IMPLEMENT_OBJECT_ALLOCATION(RecursiveDirectoryIterator)
IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(RecursiveIteratorIterator)

///////////////////////////////////////////////////////////////////////////////
// static strings

StaticString DirectoryIterator::s_class_name("directoryiterator");
StaticString
  RecursiveDirectoryIterator::s_class_name("recursivedirectoryiterator");
StaticString
  RecursiveIteratorIterator::s_class_name("recursiveiteratoriterator");

///////////////////////////////////////////////////////////////////////////////
// helper

static RecursiveIteratorIterator *
get_recursiveiteratoriterator(CObjRef obj) {
  if (!obj->o_instanceof("RecursiveIteratorIterator")) {
    throw InvalidObjectTypeException(obj->o_getClassName());
  }
  CObjRef rsrc = obj->o_get("rsrc", true, "RecursiveIteratorIterator");
  return rsrc.getTyped<RecursiveIteratorIterator>();
}

static RecursiveDirectoryIterator *
get_recursivedirectoryiterator(CObjRef obj) {
  if (!obj->o_instanceof("RecursiveDirectoryIterator")) {
    throw InvalidObjectTypeException(obj->o_getClassName());
  }
  // SplFileInfo as context -- rsrc is a private property
  CObjRef rsrc = obj->o_get("rsrc", true, "SplFileInfo");
  return rsrc.getTyped<RecursiveDirectoryIterator>();
}

static DirectoryIterator *
get_directoryiterator(CObjRef obj) {
  if (!obj->o_instanceof("DirectoryIterator")) {
    throw InvalidObjectTypeException(obj->o_getClassName());
  }
  // SplFileInfo as context -- rsrc is a private property
  CObjRef rsrc = obj->o_get("rsrc", true, "SplFileInfo");
  return rsrc.getTyped<DirectoryIterator>();
}

DirectoryIterator::DirectoryIterator(CStrRef path) :
  m_path(path), m_index(0) {
  Variant dir = f_opendir(m_path);
  if (!dir.same(false)) {
    m_dir = dir;
    m_dirEntry = f_readdir(m_dir);
    m_fileName = getPathName();
  }
}

String DirectoryIterator::getPathName() const {
  if (!m_dirEntry.same(false)) {
    String path = m_path;
    if (path.c_str()[path.size()-1] != '/') path += "/";
    return path + m_dirEntry.toString();
  }
  return String();
}

void DirectoryIterator::rewind() {
  if (!m_dir.same(NULL)) {
    f_rewinddir(m_dir);
    m_dirEntry = f_readdir(m_dir);
    m_index = 0;
    m_fileName = getPathName();
  }
}

bool DirectoryIterator::valid() {
  return !m_dirEntry.same(false) && !m_dirEntry.toString().empty();
}

void DirectoryIterator::next() {
  if (!m_dir.same(NULL)) {
    m_dirEntry = f_readdir(m_dir);
    m_index++;
    m_fileName = getPathName();
  }
}

bool DirectoryIterator::isdot() {
  return (m_dirEntry == "." || m_dirEntry == "..");
}

RecursiveDirectoryIterator::RecursiveDirectoryIterator(CStrRef path,
                                                       int flags) :
  DirectoryIterator(path), m_flags(flags) {
}

void RecursiveDirectoryIterator::rewind() {
  DirectoryIterator::rewind();
  if (isdot()) {
    next();
  }
}

void RecursiveDirectoryIterator::next() {
  do {
    DirectoryIterator::next();
  } while (valid() && DirectoryIterator::isdot());
}

RecursiveIteratorIterator::RecursiveIteratorIterator(CObjRef iterator,
  int mode, int flags) : m_iterator(iterator), m_mode(mode), m_flags(flags) {
  m_iterators.push_back(std::make_pair(iterator.get(), 0));
  iterator->incRefCount();
}

void RecursiveIteratorIterator::freeAllIterators() {
  for (IteratorList::const_iterator it = m_iterators.begin();
      it != m_iterators.end();
      ++it) {
    decRefObj(it->first);
  }
  m_iterators.clear();
}

void RecursiveIteratorIterator::sweep() {
  // Don't deref the ObjectData*'s during sweep (they will be
  // deallocated by the smart allocator, and may already be
  // deallocated).
  m_iterators.clear();
}

Object f_hphp_recursiveiteratoriterator___construct(CObjRef obj, CObjRef iterator, int64 mode, int64 flags) {
  if (iterator->o_instanceof("RecursiveDirectoryIterator")) {
    CVarRef rsrc = iterator->o_get("rsrc", true, "SplFileInfo");
    obj->o_set("rsrc", NEWOBJ(RecursiveIteratorIterator)(rsrc, mode, flags),
               "RecursiveIteratorIterator");
    return obj;
  }
  throw NotImplementedException("this type of iterator");
}

Object f_hphp_recursiveiteratoriterator_getinneriterator(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  ASSERT(size > 0);
  return size == 1 ? Object() : rii->m_iterators[size-1].first;
}

Variant f_hphp_recursiveiteratoriterator_current(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  ASSERT(size > 0);
  if (rii->m_iterator.is<RecursiveDirectoryIterator>()) {
    ObjectData* rdi = SystemLib::AllocRecursiveDirectoryIteratorObject();
    rdi->o_set("rsrc",
      rii->m_iterators[size-1].first, "SplFileInfo");
    return f_hphp_recursivedirectoryiterator_current(rdi);
  }
  throw NotImplementedException("this type of iterator");
}

Variant f_hphp_recursiveiteratoriterator_key(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  ASSERT(size > 0);
  if (rii->m_iterator.is<RecursiveDirectoryIterator>()) {
    ObjectData* rdi = SystemLib::AllocRecursiveDirectoryIteratorObject();
    rdi->o_set("rsrc",
      rii->m_iterators[size-1].first, "SplFileInfo");
    return f_hphp_recursivedirectoryiterator_key(rdi);
  }
  throw NotImplementedException("this type of iterator");
}

void f_hphp_recursiveiteratoriterator_next(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  if (!size) return;
  Object ci = rii->m_iterators[size-1].first;
  if (rii->m_mode == HPHP::q_RecursiveIteratorIterator$$SELF_FIRST) {
    if (!ci.is<RecursiveDirectoryIterator>()) {
      throw NotImplementedException("this type of iterator");
    }
    RecursiveDirectoryIterator *rdi =
      ci.getTyped<RecursiveDirectoryIterator>();
    String pathName = rdi->getPathName();
    if (f_is_dir(pathName) && !rii->m_iterators[size-1].second) {
      rii->m_iterators[size-1].second = 1;
      RecursiveDirectoryIterator *ii =
        NEWOBJ(RecursiveDirectoryIterator)(pathName, rdi->m_flags);
      rii->m_iterators.push_back(std::make_pair(ii, 0));
      ii->incRefCount();
      if (ii->isdot()) ii->next();
    } else {
      rdi->next();
      rii->m_iterators[size-1].second = 0;
    }
    if (f_hphp_recursiveiteratoriterator_valid(obj)) return;
    decRefObj(rii->m_iterators.back().first);
    rii->m_iterators.pop_back();
    return f_hphp_recursiveiteratoriterator_next(obj);
  } else if (rii->m_mode == HPHP::q_RecursiveIteratorIterator$$CHILD_FIRST ||
             rii->m_mode == HPHP::q_RecursiveIteratorIterator$$LEAVES_ONLY) {
    if (!ci.is<RecursiveDirectoryIterator>()) {
      throw NotImplementedException("this type of iterator");
    }
    RecursiveDirectoryIterator *rdi =
      ci.getTyped<RecursiveDirectoryIterator>();
    String pathName = rdi->getPathName();
    if (pathName.empty()) {
      decRefObj(rii->m_iterators.back().first);
      rii->m_iterators.pop_back();
      return f_hphp_recursiveiteratoriterator_next(obj);
    } else if (f_is_dir(pathName)) {
      if (!rii->m_iterators[size-1].second) {
        rii->m_iterators[size-1].second = 1;
        RecursiveDirectoryIterator *ii =
          NEWOBJ(RecursiveDirectoryIterator)(pathName, rdi->m_flags);
        rii->m_iterators.push_back(std::make_pair(ii, 0));
        ii->incRefCount();
        ii->rewind();
        if (f_hphp_recursiveiteratoriterator_valid(obj)) return;
        return f_hphp_recursiveiteratoriterator_next(obj);
      } else {
        // CHILD_FIRST: 0 - drill down; 1 - visit 2 - next
        // LEAVES_ONLY: 0 - drill down; 1 - next
        if (rii->m_mode == HPHP::q_RecursiveIteratorIterator$$CHILD_FIRST &&
          rii->m_iterators[size-1].second == 1) {
          rii->m_iterators[size-1].second = 2;
          return;
        }
      }
    }
    rii->m_iterators[size-1].second = 0;
    rdi->next();
    if (f_hphp_recursiveiteratoriterator_valid(obj)) return;
    return f_hphp_recursiveiteratoriterator_next(obj);
  } else {
    if (!ci.is<RecursiveDirectoryIterator>()) {
      throw NotImplementedException("this type of iterator");
    }
    RecursiveDirectoryIterator *rdi =
      ci.getTyped<RecursiveDirectoryIterator>();
    ASSERT(rii->m_iterators[size-1].second == 0);
    rdi->next();
  }
}

void f_hphp_recursiveiteratoriterator_rewind(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  rii->freeAllIterators();
  rii->m_iterators.push_back(std::make_pair(rii->m_iterator.get(), 0));
  rii->m_iterator->incRefCount();
  if (rii->m_iterator.is<RecursiveDirectoryIterator>()) {
    ObjectData* rdi = SystemLib::AllocRecursiveDirectoryIteratorObject();
    rdi->o_set("rsrc", rii->m_iterator, "SplFileInfo");
    f_hphp_recursivedirectoryiterator_rewind(rdi);
    if (!f_hphp_recursiveiteratoriterator_valid(obj)) {
      f_hphp_recursiveiteratoriterator_next(obj);
    }
    return;
  }
  throw NotImplementedException("this type of iterator");
}

bool f_hphp_recursiveiteratoriterator_valid(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  if (!size) return false;
  Object firstIt = rii->m_iterators[size-1].first;
  if (firstIt.is<RecursiveDirectoryIterator>()) {
    RecursiveDirectoryIterator* rdi =
      firstIt.getTyped<RecursiveDirectoryIterator>();
    bool valid = rdi->valid();
    if (valid) {
      if (rii->m_mode == HPHP::q_RecursiveIteratorIterator$$LEAVES_ONLY ||
          rii->m_mode == HPHP::q_RecursiveIteratorIterator$$CHILD_FIRST) {
        String pathName = rdi->getPathName();
        if (f_is_dir(pathName)) {
          if (rii->m_iterators[size-1].second > 0 &&
              rii->m_mode == HPHP::q_RecursiveIteratorIterator$$CHILD_FIRST) {
            return true;
          }
          return false;
        }
      }
    }
    return valid;
  }
  throw NotImplementedException("this type of iterator");
}

bool f_hphp_directoryiterator___construct(CObjRef obj, CStrRef path) {
  SmartObject<DirectoryIterator> rsrc = NEWOBJ(DirectoryIterator)(path);
  obj->o_set("rsrc", rsrc, "SplFileInfo");
  return !rsrc->m_dir.isNull();
}

Variant f_hphp_directoryiterator_key(CObjRef obj) {
  DirectoryIterator *di = get_directoryiterator(obj);
  return di->m_index;
}

void f_hphp_directoryiterator_next(CObjRef obj) {
  DirectoryIterator *di = get_directoryiterator(obj);
  di->next();
}

void f_hphp_directoryiterator_rewind(CObjRef obj) {
  DirectoryIterator *di = get_directoryiterator(obj);
  di->rewind();
}

void f_hphp_directoryiterator_seek(CObjRef obj, int64 position) {
  DirectoryIterator *di = get_directoryiterator(obj);
  for (int i = 0; i < position - di->m_index; i++) {
    f_hphp_directoryiterator_next(obj);
  }
}

Variant f_hphp_directoryiterator_current(CObjRef obj) {
  return obj;
}

String f_hphp_directoryiterator___tostring(CObjRef obj) {
  DirectoryIterator *di = get_directoryiterator(obj);
  return di->m_dirEntry;
}

bool f_hphp_directoryiterator_valid(CObjRef obj) {
  DirectoryIterator *di = get_directoryiterator(obj);
  return di->valid();
}

bool f_hphp_directoryiterator_isdot(CObjRef obj) {
  DirectoryIterator *di = get_directoryiterator(obj);
  return di->isdot();
}

bool f_hphp_recursivedirectoryiterator___construct(CObjRef obj, CStrRef path,
    int64 flags) {
  SmartObject<RecursiveDirectoryIterator> rsrc =
    NEWOBJ(RecursiveDirectoryIterator)(path, flags);
  obj->o_set("rsrc", rsrc, "SplFileInfo");
  return !rsrc->m_dir.isNull();
}

Variant f_hphp_recursivedirectoryiterator_key(CObjRef obj) {
  RecursiveDirectoryIterator *rdi = get_recursivedirectoryiterator(obj);
  if (rdi->m_flags == HPHP::q_RecursiveDirectoryIterator$$KEY_AS_FILENAME) {
    return rdi->m_dirEntry;
  }
  return rdi->getPathName();
}

void f_hphp_recursivedirectoryiterator_next(CObjRef obj) {
  RecursiveDirectoryIterator *rdi = get_recursivedirectoryiterator(obj);
  rdi->next();
}

void f_hphp_recursivedirectoryiterator_rewind(CObjRef obj) {
  f_hphp_directoryiterator_rewind(obj);
  if (f_hphp_directoryiterator_isdot(obj)) {
    f_hphp_recursivedirectoryiterator_next(obj);
  }
}

void f_hphp_recursivedirectoryiterator_seek(CObjRef obj, int64 position) {
  f_hphp_directoryiterator_seek(obj, position);
}

String f_hphp_recursivedirectoryiterator___tostring(CObjRef obj) {
  return f_hphp_directoryiterator___tostring(obj);
}

bool f_hphp_recursivedirectoryiterator_valid(CObjRef obj) {
  return f_hphp_directoryiterator_valid(obj);
}

Variant f_hphp_recursivedirectoryiterator_current(CObjRef obj) {
  RecursiveDirectoryIterator *rdi = get_recursivedirectoryiterator(obj);
  String pathName = rdi->getPathName();
  if (rdi->m_flags & HPHP::q_RecursiveDirectoryIterator$$CURRENT_AS_PATHNAME) {
    return pathName;
  }
  if (rdi->m_flags & HPHP::q_RecursiveDirectoryIterator$$CURRENT_AS_FILEINFO) {
    ObjectData* c_splfi = SystemLib::AllocSplFileInfoObject();
    c_splfi->o_set("rsrc", NEWOBJ(SplFileInfo)(pathName), "SplFileInfo");
    return c_splfi;
  }
  return obj;
}

bool f_hphp_recursivedirectoryiterator_haschildren(CObjRef obj) {
  return f_hphp_splfileinfo_isdir(obj);
}

Object f_hphp_recursivedirectoryiterator_getchildren(CObjRef obj) {
  if (!f_hphp_recursivedirectoryiterator_haschildren(obj)) return Object();
  RecursiveDirectoryIterator *rdi = get_recursivedirectoryiterator(obj);
  ObjectData* o_rdi = SystemLib::AllocRecursiveDirectoryIteratorObject();
  o_rdi->o_set("rsrc",
               NEWOBJ(RecursiveDirectoryIterator)(rdi->getPathName(),
                                                  rdi->m_flags),
               "SplFileInfo");
  return o_rdi;
}

String f_hphp_recursivedirectoryiterator_getsubpath(CObjRef obj) {
  throw NotImplementedException(__func__);
}

String f_hphp_recursivedirectoryiterator_getsubpathname(CObjRef obj) {
  throw NotImplementedException(__func__);
}

c_MutableArrayIterator::c_MutableArrayIterator(const ObjectStaticCallbacks *cb /* = &cw_MIterCtx */)
    : ExtObjectData(cb), m_valid(false) {
}

c_MutableArrayIterator::~c_MutableArrayIterator() {
  c_MutableArrayIterator::sweep();
}

void c_MutableArrayIterator::sweep() {
  if (m_valid) {
    MIterCtx& mi = marr();
    mi.~MIterCtx();
    m_valid = false;
  }
}

void c_MutableArrayIterator::t___construct(VRefParam array) {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::__construct);
  if (m_valid) {
    MIterCtx& mi = marr();
    mi.~MIterCtx();
    m_valid = false;
  }
  Variant var(strongBind(array));
  TypedValue* tv = (TypedValue*)(&var);
  ASSERT(tv->m_type == KindOfRef);
  TypedValue* rtv = tv->m_data.pref->tv();
  if (rtv->m_type == KindOfArray) {
    MIterCtx& mi = marr();
    (void) new (&mi) MIterCtx(tv->m_data.pref);
    m_valid = mi.mArray().advance();
    if (!m_valid) mi.~MIterCtx();
  } else if (rtv->m_type == KindOfObject) {
    CStrRef ctxStr = hhvm
                     ? g_vmContext->getContextClassName()
                     : FrameInjection::GetClassName(true);
    if (rtv->m_data.pobj->isCollection()) {
      raise_error("Collection elements cannot be taken by reference");
    }
    bool isIterator;
    Object obj = rtv->m_data.pobj->iterableObject(isIterator);
    if (isIterator) {
      raise_error("An iterator cannot be used with foreach by reference");
    }
    Array iterArray = obj->o_toIterArray(ctxStr, true);
    ArrayData* ad = iterArray.detach();
    MIterCtx& mi = marr();
    (void) new (&mi) MIterCtx(ad);
    m_valid = mi.mArray().advance();
    if (!m_valid) mi.~MIterCtx();
  } else {
    raise_warning("Invalid argument supplied for foreach()");
  }
}

Variant c_MutableArrayIterator::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::__destruct);
  return null;
}

Variant c_MutableArrayIterator::t_currentref() {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::currentref);
  if (!m_valid) return null;
  MIterCtx& mi = marr();
  return strongBind(*(Variant*)(&mi.val()));
}

Variant c_MutableArrayIterator::t_current() {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::current);
  if (!m_valid) return null;
  MIterCtx& mi = marr();
  return *(const Variant*)(&mi.val());
}

Variant c_MutableArrayIterator::t_key() {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::key);
  if (!m_valid) return false;
  MIterCtx& mi = marr();
  return *(Variant*)(&mi.key());
}

void c_MutableArrayIterator::t_next() {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::next);
  if (!m_valid) return;
  MIterCtx &mi = marr();
  if (!mi.mArray().advance()) {
    mi.~MIterCtx();
    m_valid = false;
  }
}

bool c_MutableArrayIterator::t_valid() {
  INSTANCE_METHOD_INJECTION_BUILTIN(MutableArrayIterator, MutableArrayIterator::valid);
  return m_valid;
}

///////////////////////////////////////////////////////////////////////////////
}
