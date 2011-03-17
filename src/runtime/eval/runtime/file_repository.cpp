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
#include <util/atomic.h>
#include <runtime/eval/runtime/eval_state.h>

using namespace std;

namespace HPHP {

extern bool (*file_dump)(const char *filename);

namespace Eval {
///////////////////////////////////////////////////////////////////////////////

set<string> FileRepository::s_names;

PhpFile::PhpFile(StatementPtr tree, const vector<StaticStatementPtr> &statics,
                 const struct stat &s)
  : Block(statics), m_refCount(1), m_timestamp(s.st_mtime),
    m_ino(s.st_ino), m_devId(s.st_dev), m_tree(tree),
    m_profName(string("run_init::") + string(m_tree->loc()->file)) {
}

PhpFile::~PhpFile() {
  ASSERT(m_refCount == 0);
}

Variant PhpFile::eval(LVariableTable *vars) {
  NestedVariableEnvironment env(vars, *this);
  DECLARE_THREAD_INFO_NOINIT
  EvalFrameInjection fi(empty_string, m_profName.c_str(), env,
      m_tree->loc()->file, NULL, FrameInjection::PseudoMain);
  restart:
  try {
    m_tree->eval(env);
  } catch (GotoException &e) {
    goto restart;
  } catch (UnlimitedGotoException &e) {
    goto restart;
  }
  if (env.isGotoing()) {
    throw FatalErrorException(0, "Unable to reach goto label %s",
                              env.getGoto().c_str());
  }
  if (env.isReturning()) {
    return env.getRet();
  } else if (env.isBreaking()) {
    throw FatalErrorException("Cannot break/continue out of a file");
  }
  return true;
}

void PhpFile::decRef() {
  ASSERT(m_refCount);
  if (atomic_dec(m_refCount) == 0) {
    delete this;
  }
}

void PhpFile::incRef() {
  atomic_inc(m_refCount);
}

bool PhpFile::isChanged(const struct stat &s) {
  return m_timestamp < s.st_mtime || m_ino != s.st_ino || m_devId != s.st_dev;
}

Mutex FileRepository::s_lock;
hphp_hash_map<std::string, PhpFile*, string_hash>
FileRepository::s_files;

static class FileDumpInitializer {
  public: FileDumpInitializer() {
    file_dump = FileRepository::fileDump;
  }
} s_fileDumpInitializer;

bool FileRepository::fileDump(const char *filename) {
  std::ofstream out(filename);
  if (out.fail()) return false;
  Lock lock(s_lock);
  for (hphp_hash_map<string, PhpFile*, string_hash>::const_iterator it =
       s_files.begin(); it != s_files.end(); it++) {
    out << it->first.c_str() << endl;
  }
  out.close();
  return true;
}

PhpFile *FileRepository::checkoutFile(const std::string &rname,
                                      const struct stat &s) {
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
    s_files.find(name);
  if (it == s_files.end()) {
    ret = readFile(name, s);
    if (ret) {
      s_files[name] = ret;
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

bool FileRepository::findFile(std::string &path, struct stat &s) {
  if (path.empty()) return false;
  if (path[0] != '/') {
    if (!g_context->getIncludePathArray().isNull()) {
      for (ArrayIter iter(g_context->getIncludePathArray()); iter; ++iter) {
        string ip = iter.second().toString().data();
        string p;
        if (!ip.empty() && ip[ip.length() - 1] == '/') {
          p = ip + path;
        } else {
          p = ip + "/" + path;
        }
        if (fileStat(p, s) && !S_ISDIR(s.st_mode)) {
          path = p;
          return true;
        }
      }
    }
    string cwd = g_context->getCwd().data();
    if (!cwd.empty() && cwd[cwd.length() - 1] == '/') {
      path = cwd + path;
    } else {
      path = cwd + "/" + path;
    }
  }
  return fileStat(path, s) && !S_ISDIR(s.st_mode);
}

PhpFile *FileRepository::readFile(const std::string &name,
                                  const struct stat &s) {
  vector<StaticStatementPtr> sts;
  const char *canoname = canonicalize(name);
  StatementPtr stmt = Parser::ParseFile(canoname, sts);
  if (stmt) {
    PhpFile *p = new PhpFile(stmt, sts, s);
    return p;
  }
  return NULL;
}

bool FileRepository::fileStat(const std::string &name, struct stat &s) {
  return stat(name.c_str(), &s) == 0;
}

const char* FileRepository::canonicalize(const std::string &name) {
  return s_names.insert(name).first->c_str();
}

///////////////////////////////////////////////////////////////////////////////
}
}
