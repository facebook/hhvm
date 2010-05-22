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

#include <runtime/base/source_info.h>
#include <runtime/base/externals.h>
#include <util/lock.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SourceInfo SourceInfo::TheSourceInfo;
SourceInfoHook *SourceInfo::s_hook = NULL;

SourceInfo::SourceInfo() : m_loaded(false), m_source_root_len(0) {
}

void SourceInfo::load() {
  Lock lock(m_mutex);
  if (!m_loaded) {
    loadImpl(m_cpp2php,   g_source_info);
    loadImpl(m_cls2file,  m_file2cls,  g_source_cls2file);
    loadImpl(m_func2file, m_file2func, g_source_func2file);
    m_source_root_len = strlen(g_source_root);
    m_loaded = true;
  }
}

bool SourceInfo::translate(StackTrace::FramePtr f) {
  if (!m_loaded) load();

  const char *file = f->filename.c_str();
  if (strncmp(g_source_root, file, m_source_root_len) == 0) {
    char key[256];
    snprintf(key, sizeof(key), "%s:%d", file + m_source_root_len, f->lineno);
    LocationMap::const_iterator iter = m_cpp2php.find(key);
    if (iter != m_cpp2php.end()) {
      f->filename = iter->second->file;
      f->lineno = iter->second->line;
      return true;
    }
  }

  return false;
}

void SourceInfo::getDeclaredFunctions(const char *filename,
                                      std::vector<const char *> &functions) {
  if (!m_loaded) load();

  functions.clear();
  NameMap::const_iterator iter = m_file2func.find(filename);
  if (iter != m_file2func.end()) {
    functions = iter->second;
  }
}

void SourceInfo::getDeclaredClasses(const char *filename,
                                    std::vector<const char *> &classes) {
  if (!m_loaded) load();

  classes.clear();
  NameMap::const_iterator iter = m_file2cls.find(filename);
  if (iter != m_file2cls.end()) {
    classes = iter->second;
  }
}

const char *SourceInfo::getClassDeclaringFile(const char *name) {
  if (!m_loaded) load();
  if (s_hook) {
    const char *file = s_hook->getClassDeclaringFile(name);
    if (file) return file;
  }
  INameMap::const_iterator iter = m_cls2file.find(name);
  if (iter != m_cls2file.end()) {
    return iter->second[0];
  }
  return NULL;
}

const char *SourceInfo::getFunctionDeclaringFile(const char *name) {
  if (!m_loaded) load();
  if (s_hook) {
    const char *file = s_hook->getFunctionDeclaringFile(name);
    if (file) return file;
  }
  INameMap::const_iterator iter = m_func2file.find(name);
  if (iter != m_func2file.end()) {
    return iter->second[0];
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void SourceInfo::loadImpl(LocationMap &dest, const char **p) {
  while (*p) {
    const char *source = *p++;
    const char *file = *p++;
    int line = (int)(long)(*p++);

    LocationInfo *loc = new LocationInfo();
    loc->file = file;
    loc->line = line;

    ASSERT(dest.find(source) == dest.end());
    dest[source] = loc;
  }
}

void SourceInfo::loadImpl(INameMap &forward, NameMap &backward, const char **p){
  while (*p) {
    const char *name = *p++;
    const char *file = *p++;
    forward[name].push_back(file);
    backward[file].push_back(name);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
