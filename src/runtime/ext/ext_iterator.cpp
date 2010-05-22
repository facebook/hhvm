/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJECT_ALLOCATION(DirectoryIterator);
IMPLEMENT_OBJECT_ALLOCATION(RecursiveDirectoryIterator);
IMPLEMENT_OBJECT_ALLOCATION(RecursiveIteratorIterator);

// helper
static RecursiveIteratorIterator *
get_recursiveiteratoriterator(CObjRef obj) {
  c_recursiveiteratoriterator *c_rii =
    obj.getTyped<c_recursiveiteratoriterator>();
  return c_rii->m_rsrc.toObject().getTyped<RecursiveIteratorIterator>();
}

static RecursiveDirectoryIterator *
get_recursivedirectoryiterator(CObjRef obj) {
  c_recursivedirectoryiterator *c_rdi =
    obj.getTyped<c_recursivedirectoryiterator>();
  return c_rdi->m_rsrc.toObject().getTyped<RecursiveDirectoryIterator>();
}

static DirectoryIterator *
get_directoryiterator(CObjRef obj) {
  c_directoryiterator *c_di =
    obj.getTyped<c_directoryiterator>();
  return c_di->m_rsrc.toObject().getTyped<DirectoryIterator>();
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

String DirectoryIterator::getPathName() {
  if (!m_dirEntry.same(false)) {
    std::string path = m_path;
    if (path.c_str()[path.size()-1] != '/') path += "/";
    return String(path) + m_dirEntry.toString();
  }
  return String();
}

void DirectoryIterator::rewind() {
  if (!m_dir.same(false)) {
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
  if (!m_dir.same(false)) {
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
  m_iterators.push_back(std::pair<Object, bool>(m_iterator, 0));
}

Object f_hphp_recursiveiteratoriterator___construct(CObjRef obj, CObjRef iterator, int64 mode, int64 flags) {
  c_recursiveiteratoriterator *c_rii =
    obj.getTyped<c_recursiveiteratoriterator>();
  if (iterator.is<c_recursivedirectoryiterator>()) {
    c_recursivedirectoryiterator *c_rdi =
      iterator.getTyped<c_recursivedirectoryiterator>();
    c_rii->m_rsrc = NEW(RecursiveIteratorIterator)(c_rdi->m_rsrc, mode, flags);
    return c_rii;
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
    c_recursivedirectoryiterator *c_rdi =
      NEW(c_recursivedirectoryiterator)();
    c_rdi->m_rsrc = rii->m_iterators[size-1].first;
    return f_hphp_recursivedirectoryiterator_current(c_rdi);
  }
  throw NotImplementedException("this type of iterator");
}

Variant f_hphp_recursiveiteratoriterator_key(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  ASSERT(size > 0);
  if (rii->m_iterator.is<RecursiveDirectoryIterator>()) {
    c_recursivedirectoryiterator *c_rdi =
      NEW(c_recursivedirectoryiterator)();
    c_rdi->m_rsrc = rii->m_iterators[size-1].first;
    return f_hphp_recursivedirectoryiterator_key(c_rdi);
  }
  throw NotImplementedException("this type of iterator");
}

void f_hphp_recursiveiteratoriterator_next(CObjRef obj) {
  RecursiveIteratorIterator *rii = get_recursiveiteratoriterator(obj);
  unsigned int size = rii->m_iterators.size();
  if (!size) return;
  Object ci = rii->m_iterators[size-1].first;
  if (rii->m_mode == HPHP::q_recursiveiteratoriterator_SELF_FIRST) {
    if (!ci.is<RecursiveDirectoryIterator>()) {
      throw NotImplementedException("this type of iterator");
    }
    RecursiveDirectoryIterator *rdi =
      ci.getTyped<RecursiveDirectoryIterator>();
    String pathName = rdi->getPathName();
    if (f_is_dir(pathName) && !rii->m_iterators[size-1].second) {
      rii->m_iterators[size-1].second = 1;
      RecursiveDirectoryIterator *ii =
        NEW(RecursiveDirectoryIterator)(pathName, rdi->m_flags);
      rii->m_iterators.push_back(std::pair<Object, bool>(ii, 0));
      if (ii->isdot()) ii->next();
    } else {
      rdi->next();
      rii->m_iterators[size-1].second = 0;
    }
    if (f_hphp_recursiveiteratoriterator_valid(obj)) return;
    rii->m_iterators.pop_back();
    return f_hphp_recursiveiteratoriterator_next(obj);
  } else if (rii->m_mode == HPHP::q_recursiveiteratoriterator_CHILD_FIRST ||
             rii->m_mode == HPHP::q_recursiveiteratoriterator_LEAVES_ONLY) {
    if (!ci.is<RecursiveDirectoryIterator>()) {
      throw NotImplementedException("this type of iterator");
    }
    RecursiveDirectoryIterator *rdi =
      ci.getTyped<RecursiveDirectoryIterator>();
    String pathName = rdi->getPathName();
    if (pathName.empty()) {
      rii->m_iterators.pop_back();
      return f_hphp_recursiveiteratoriterator_next(obj);
    } else if (f_is_dir(pathName)) {
      if (!rii->m_iterators[size-1].second) {
        rii->m_iterators[size-1].second = 1;
        RecursiveDirectoryIterator *ii =
          NEW(RecursiveDirectoryIterator)(pathName, rdi->m_flags);
        rii->m_iterators.push_back(std::pair<Object, bool>(ii, 0));
        ii->rewind();
        if (f_hphp_recursiveiteratoriterator_valid(obj)) return;
        return f_hphp_recursiveiteratoriterator_next(obj);
      } else {
        // CHILD_FIRST: 0 - drill down; 1 - visit 2 - next
        // LEAVES_ONLY: 0 - drill down; 1 - next
        if (rii->m_mode == HPHP::q_recursiveiteratoriterator_CHILD_FIRST &&
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
  rii->m_iterators.clear();
  rii->m_iterators.push_back(std::pair<Object,bool>(rii->m_iterator, 0));
  if (rii->m_iterator.is<RecursiveDirectoryIterator>()) {
    c_recursivedirectoryiterator *c_rdi =
      NEW(c_recursivedirectoryiterator)();
    c_rdi->m_rsrc = rii->m_iterator;
    f_hphp_recursivedirectoryiterator_rewind(c_rdi);
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
  if (rii->m_iterators[size-1].first.is<RecursiveDirectoryIterator>()) {
    RecursiveDirectoryIterator *rdi =
      rii->m_iterators[size-1].first.getTyped<RecursiveDirectoryIterator>();
    bool valid = rdi->valid();
    if (valid) {
      if (rii->m_mode == HPHP::q_recursiveiteratoriterator_LEAVES_ONLY ||
          rii->m_mode == HPHP::q_recursiveiteratoriterator_CHILD_FIRST) {
        String pathName = rdi->getPathName();
        if (f_is_dir(pathName)) {
          if (rii->m_iterators[size-1].second > 0 &&
              rii->m_mode == HPHP::q_recursiveiteratoriterator_CHILD_FIRST) {
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

Object f_hphp_directoryiterator___construct(CObjRef obj, CStrRef path) {
  c_directoryiterator *c_di = obj.getTyped<c_directoryiterator>();
  c_di->m_rsrc = NEW(DirectoryIterator)(path);
  return c_di;
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

Object f_hphp_recursivedirectoryiterator___construct(CObjRef obj, CStrRef path, int64 flags) {
  c_recursivedirectoryiterator *c_rdi =
    obj.getTyped<c_recursivedirectoryiterator>();
  c_rdi->m_rsrc = NEW(RecursiveDirectoryIterator)(path, flags);
  return c_rdi;
}

Variant f_hphp_recursivedirectoryiterator_key(CObjRef obj) {
  RecursiveDirectoryIterator *rdi = get_recursivedirectoryiterator(obj);
  if (rdi->m_flags == HPHP::q_recursivedirectoryiterator_KEY_AS_FILENAME) {
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
  if (rdi->m_flags & HPHP::q_recursivedirectoryiterator_CURRENT_AS_PATHNAME) {
    return pathName;
  }
  if (rdi->m_flags & HPHP::q_recursivedirectoryiterator_CURRENT_AS_FILEINFO) {
    c_splfileinfo *c_splfi = NEW(c_splfileinfo)();
    c_splfi->m_rsrc = NEW(SplFileInfo)(pathName);
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
  c_recursivedirectoryiterator *c_rdi = NEW(c_recursivedirectoryiterator)();
  c_rdi->m_rsrc =
    NEW(RecursiveDirectoryIterator)(rdi->getPathName(), rdi->m_flags);
  return c_rdi;
}

String f_hphp_recursivedirectoryiterator_getsubpath(CObjRef obj) {
  throw NotImplementedException(__func__);
}

String f_hphp_recursivedirectoryiterator_getsubpathname(CObjRef obj) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////
}
