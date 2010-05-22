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

#ifndef __DEPENDENCY_GRAPH_H__
#define __DEPENDENCY_GRAPH_H__

#include <compiler/hphp.h>
#include <util/json.h>
#include <util/db_query.h>
#include <util/db_conn.h>
#include <compiler/hphp_unique.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ServerData);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(CodeError);
DECLARE_BOOST_TYPES(Dependency);
DECLARE_BOOST_TYPES(DependencyGraph);

class Dependency : public JSON::ISerializable {
public:
  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;

public:
  Dependency();

  int m_programCount;
  std::vector<std::string> m_programs;
  ConstructPtr m_parent;
  ConstructPtr m_child;
};

/**
 * Some dependencies are added during parse (phase 1).
 * Some dependencies are added independent of programs (phase 2).
 * Some dependencies are added under context of programs (phase 3).
 * Phase 2 dependencies are resolved in phase 3.
 * Phase 1 dependencies don't need resolution at all, as they are file level
 * ones.
 */
class DependencyGraph : public JSON::ISerializable {
  friend class DependencyGraphHook;
public:
  enum KindOf {
#define DEPENDENCY_ENTRY(x) KindOf ## x,
#include "compiler/analysis/dependency.inc"
#undef DEPENDENCY_ENTRY
    KindOfCount
  };

public:
  DependencyGraph();

  /**
   * Add file dependencies: PHP, CSS or JS.
   *
   * Returns the PHP file name it's being included, if any.
   */
  std::string add(KindOf kindOf, ConstructPtr childExp,
                  ExpressionPtr parentExp, CodeErrorPtr codeError,
                  bool documentRoot = false);

  /**
   * Add unresolved KindOfClassDerivation or KindOfFunctionCall.
   */
  void add(KindOf kindOf,
           const std::string &childName, ConstructPtr child,
           const std::string &parentName);

  /**
   * Add Program dependencies: PrgramMaxInclude, ProgramMinInclude,
   * ProgramUserFunction or ProgramUserClass
   */
  void add(KindOf kindOf, const std::string &program,
           const std::string &parent, StatementPtr stmt);

  /**
   * Add fully resolved dependencies. Returns true if newly added,
   * false if it's there already.
   */
  void add(KindOf kindOf, const std::string &program,
           const std::string &childName, ConstructPtr child,
           const std::string &parentName, ConstructPtr parent,
           CodeErrorPtr codeError = CodeErrorPtr());

  /**
   * These are all potential parents. With this list, we can then build
   * list of orphaned parents.
   */
  void addParent(KindOf kindOf, const std::string &program,
                 const std::string &parentName, ConstructPtr parent);

  /**
   * Whether there are any dependents.
   */
  bool hasAnyChildren(KindOf kindOf, const std::string &parent) const;

  /**
   * This will get all children (not just direct ones) of a parent
   * recursively.
   */
  const StringToConstructPtrMap&
  getAllChildren(KindOf kindOf, const std::string &parent) const;

  /**
   * This will get all parents (not just direct ones) of a child
   * recursively.
   */
  const StringToConstructPtrMap&
  getAllParents(KindOf kindOf, const std::string &child) const;

  /**
   * Get direct children.
   */
  void getChildren(KindOf kindOf, const std::string &parent,
                   DependencyPtrVec &children) const;

  /**
   * Look up a parent.
   */
  ConstructPtr getParent(KindOf kindOf, const std::string &name) const;

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;

  /**
   * Save one type of dependency as one .js file.
   */
  void saveToFiles(const char *dir) const;

  /**
   * Save dependency data to a database.
   */
  void saveToDB(ServerDataPtr server, int runId) const;

  bool checkCircle(KindOf kindOf,
                   const std::string &childName,
                   const std::string &parentName);

  static std::string parseInclude(const std::string &source,
                                  ExpressionPtr exp,
                                  bool documentRoot = false);

  static void setHookHandler(bool (*hookHandler)(DependencyGraph *depGraph,
                                                 KindOf kindOf,
                                                 ConstructPtr childExp,
                                                 ExpressionPtr parentExp,
                                                 CodeErrorPtr codeError,
                                                 bool documentRoot,
                                                 std::string &child,
                                                 std::string &parent,
                                                 HphpHookUniqueId id)) {
    m_hookHandler = hookHandler;
  }

private:
  static std::vector<const char *> DependencyTexts;
  static std::vector<const char *> &getDependencyTexts();

  typedef boost::shared_ptr<DependencyPtrVec> DependencyPtrVecPtr;
  typedef std::map<std::string, DependencyPtrVecPtr, stdltstr> DependencyMap;
  typedef std::map<std::string, DependencyMap, stdltstr> DependencyMapMap;
  typedef DependencyMap::const_iterator MapConstIter;
  typedef DependencyMapMap::const_iterator MapMapConstIter;
  typedef std::map<std::string, StringToConstructPtrMap, stdltstr>
  StringConstructMapMap;

  std::vector<DependencyMapMap> m_forwards; // parent -> child
  std::vector<DependencyMapMap> m_reverses; // child -> parent
  std::vector<StringToDependencyPtrMap> m_parents;
  int m_total;

  // cached recursive children
  mutable std::vector<StringConstructMapMap> m_allChildren;
  mutable std::vector<StringConstructMapMap> m_allParents;

  static std::string getIncludeFilePath(const std::string &source,
                                        std::string expText,
                                        bool documentRoot);
  static void dumpMapMap(std::string &out,
                         const std::vector<DependencyMapMap> &dmms);

  void clearCache(KindOf kindOf);

  bool checkInclude(ConstructPtr childExp, ExpressionPtr parentExp,
                    CodeErrorPtr codeError, bool documentRoot,
                    std::string &child, std::string &parent);


  // hook
  static bool (*m_hookHandler)(DependencyGraph *depGraph, KindOf kindOf,
                               ConstructPtr childExp, ExpressionPtr parentExp,
                               CodeErrorPtr codeError, bool documentRoot,
                               std::string &child, std::string &parent,
                               HphpHookUniqueId id);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __DEPENDENCY_GRAPH_H__
