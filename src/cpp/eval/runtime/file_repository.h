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

#ifndef __EVAL_FILE_REPOSITORY_H__
#define __EVAL_FILE_REPOSITORY_H__

#include <cpp/eval/base/eval_base.h>
#include <cpp/eval/analysis/block.h>
#include <time.h>
#include <util/lock.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Statement);
DECLARE_AST_PTR(StaticStatement);

class PhpFile : public Block {
public:
  PhpFile(StatementPtr tree, const std::vector<StaticStatementPtr> &statics,
          Mutex &lock);
  ~PhpFile();
  Variant eval(LVariableTable *env);
  void decRef();
  void incRef();
  time_t readTime() const { return m_timestamp; }
private:
  Mutex &m_lock;
  int m_refCount;
  time_t m_timestamp;
  StatementPtr m_tree;
  std::string m_profName;
};

/**
 * FileRepository is global.
 */
class FileRepository {
public:
  /**
   * The first time you attempt to invoke a file in a request, this is called.
   * From then on, invoke_file will store the PhpFile and use that.
   */
  static PhpFile *checkoutFile(const std::string &name, time_t t);
  static bool findFile(std::string &path, time_t &modTime,
                       const char *currentDir);
private:
  static Mutex s_lock;
  static hphp_hash_map<std::string, PhpFile*, string_hash> m_files;
  static Mutex s_locks[128];

  static PhpFile *readFile(const std::string &name);
  static bool modifyTime(const std::string &name, time_t &time);
  static std::set<std::string> s_names;

  static const char* canonicalize(const std::string &n);
};


///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_FILE_REPOSITORY_H__ */
