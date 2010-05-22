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

#include <sys/stat.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/base/runtime_option.h>
#include <util/process.h>
#include <runtime/eval/runtime/eval_state.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

set<string> FileRepository::s_names;

PhpFile::PhpFile(StatementPtr tree, const vector<StaticStatementPtr> &statics,
                 Mutex &lock, const struct stat &s)
  : Block(statics), m_lock(lock), m_refCount(1), m_timestamp(s.st_mtime),
    m_ino(s.st_ino), m_devId(s.st_dev), m_tree(tree),
    m_profName(string("run_init::") + string(m_tree->loc()->file)) {
}

PhpFile::~PhpFile() {
  ASSERT(m_refCount == 0);
}

Variant PhpFile::eval(LVariableTable *vars) {
  NestedVariableEnvironment env(vars, *this);
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_profName.c_str());
#endif
  EvalFrameInjection fi("", m_profName.c_str(), env, m_tree->loc()->file,
      NULL, FrameInjection::PseudoMain);
  m_tree->eval(env);
  if (env.isReturning()) {
    return env.getRet();
  } else if (env.isBreaking()) {
    throw FatalErrorException("Cannot break/continue out of a file");
  }
  return true;
}

void PhpFile::decRef() {
  m_lock.lock();
  --m_refCount;
  if (m_refCount == 0) {
    m_lock.unlock();
    delete this;
    return;
  }
  m_lock.unlock();
}

void PhpFile::incRef() {
  m_lock.lock();
  ++m_refCount;
  m_lock.unlock();
}

bool PhpFile::isChanged(const struct stat &s) {
  return m_timestamp < s.st_mtime || m_ino != s.st_ino || m_devId != s.st_dev;
}

Mutex FileRepository::s_lock;
Mutex FileRepository::s_locks[128];
hphp_hash_map<std::string, PhpFile*, string_hash>
FileRepository::m_files;

PhpFile *FileRepository::checkoutFile(const std::string &rname, const struct stat &s) {
  PhpFile *ret = NULL;
  Lock lock(s_lock);
  string name;

  if (rname[0] == '/') {
    name = rname;
  } else if (RuntimeOption::SourceRoot.empty()) {
    name = Process::GetCurrentDirectory() + "/" + rname;
  } else {
    name = RuntimeOption::SourceRoot + "/" + rname;
  }

  hphp_hash_map<string, PhpFile*, string_hash>::iterator it =
    m_files.find(name);
  if (it == m_files.end()) {
    ret = readFile(name, s);
    if (ret) {
      m_files[name] = ret;
    }
  } else {
    if (it->second->isChanged(s)) {
      ret = readFile(name, s);
      if (ret) {
        it->second->decRef();
        it->second = ret;
      }
    } else {
      ret = it->second;
    }
  }
  if (ret) ret->incRef();
  return ret;
}

bool FileRepository::findFile(std::string &path, struct stat &s,
                              const char *currentDir) {
  // Check working directory first since that's what php does
  if (fileStat(path, s)) {
    return true;
  }
  for (vector<string>::const_iterator it =
         RuntimeOption::IncludeSearchPaths.begin();
       it != RuntimeOption::IncludeSearchPaths.end(); ++it) {
    string p;
    if ((*it)[0] == '.' && currentDir) {
      p = string(currentDir) + it->substr(1);
    } else {
      p = *it;
    }
    p += path;
    if (fileStat(p, s)) {
      path = p;
      return true;
    }
  }
  return false;
}

PhpFile *FileRepository::readFile(const std::string &name, const struct stat &s) {
  vector<StaticStatementPtr> sts;
  const char *canoname = canonicalize(name);
  StatementPtr stmt = Parser::parseFile(canoname, sts);
  if (stmt) {
    uint lock = hash_string(canoname) & 127;
    PhpFile *p = new PhpFile(stmt, sts, s_locks[lock], s);
    return p;
  }
  return NULL;
}

bool FileRepository::fileStat(const std::string &name, struct stat &s) {
  if (stat(name.c_str(), &s) == 0) {
    return true;
  }
  return false;
}

const char* FileRepository::canonicalize(const std::string &name) {
  return s_names.insert(name).first->c_str();
}

///////////////////////////////////////////////////////////////////////////////
}
}
