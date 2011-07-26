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

#ifndef __EVAL_FILE_REPOSITORY_H__
#define __EVAL_FILE_REPOSITORY_H__

#include <runtime/eval/base/eval_base.h>
#include <runtime/eval/analysis/block.h>
#include <time.h>
#include <sys/stat.h>
#include <util/lock.h>
#include <util/atomic.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Statement);
DECLARE_AST_PTR(StaticStatement);

class PhpFile : public Block {
public:
  PhpFile(StatementPtr tree, const std::vector<StaticStatementPtr> &statics,
          const Block::VariableIndices &variableIndices,
          const std::string &fileName, const std::string &srcRoot,
          const std::string &relPath, const std::string &md5);
  ~PhpFile();
  Variant eval(LVariableTable *env);
  void incRef() { atomic_inc(m_refCount); }
  int decRef() {
    assert(m_refCount);
    return atomic_dec(m_refCount);
  }
  int getRef() { return m_refCount; }
  // time_t readTime() const { return m_timestamp; }
  const StatementPtr &getTree() const { return m_tree; }
  const std::string &getFileName() const { return m_fileName; }
  const std::string &getSrcRoot() const { return m_srcRoot; }
  const std::string &getRelPath() const { return m_relPath; }
  const std::string &getMd5() const { return m_md5; }
private:
  int m_refCount;
  StatementPtr m_tree;
  std::string m_profName;
  std::string m_fileName;
  std::string m_srcRoot;
  std::string m_relPath;
  std::string m_md5;
};

class PhpFileWrapper {
public:
  PhpFileWrapper(const struct stat &s, PhpFile *phpFile) :
    m_timestamp(s.st_mtime), m_ino(s.st_ino), m_devId(s.st_dev),
    m_phpFile(phpFile) {}
  ~PhpFileWrapper() {}
  bool isChanged(const struct stat &s) {
    return m_timestamp < s.st_mtime ||
           m_ino != s.st_ino ||
           m_devId != s.st_dev;
  }
  PhpFile *getPhpFile() { return m_phpFile; }

private:
  time_t m_timestamp;
  ino_t m_ino;
  dev_t m_devId;
  PhpFile *m_phpFile;
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
  static PhpFile *checkoutFile(const std::string &name, const struct stat &s);
  static bool findFile(const std::string &path, struct stat *s);
  static bool fileDump(const char *filename);
  static PhpFile *readFile(const std::string &name, const struct stat &s,
                           bool &created);
  static String translateFileName(const std::string &file);
  static void onZeroRef(PhpFile *f);
private:
  static Mutex s_lock;
  static hphp_hash_map<std::string, PhpFileWrapper*, string_hash> s_files;
  static hphp_hash_map<std::string, PhpFile*, string_hash> s_md5Files;

  static bool fileStat(const std::string &name, struct stat *s);
  static void onDelete(PhpFile *f);
  static std::set<std::string> s_names;
  static const char* canonicalize(const std::string &n);
};


///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_FILE_REPOSITORY_H__ */
