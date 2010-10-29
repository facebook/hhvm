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

#ifndef __BLOCK_SCOPE_H__
#define __BLOCK_SCOPE_H__

#include <compiler/hphp.h>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(ConstantTable);
DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(IncludeExpression);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FileScope);

template<typename T>
struct smarter_pointer_hash {
  size_t operator() (const T &p) const {
    size_t x = (size_t)p.get();
    return (x >> 8) | (x << 56);
  }
};

typedef std::tr1::unordered_map<BlockScopeRawPtr, int,
                                smarter_pointer_hash<BlockScopeRawPtr>
                                > BlockScopeRawPtrFlagsHashMap;
typedef std::tr1::unordered_set<BlockScopeRawPtr,
                                smarter_pointer_hash<BlockScopeRawPtr>
                                > BlockScopeRawPtrHashSet;

typedef std::list<BlockScopeRawPtr> BlockScopeRawPtrQueue;

/**
 * Base class of ClassScope and FunctionScope.
 */
class BlockScope : public boost::enable_shared_from_this<BlockScope> {
public:
  enum KindOf {
    ClassScope,
    FunctionScope,
    FileScope,
    ProgramScope,
  };

  enum UseKinds {
    UseKindCaller = 1,
    UseKindStaticRef = 2,
    UseKindNonStaticRef = 4,
    UseKindConstRef = 8,
    UseKindParentRef = 16,
    UseKindAny = (unsigned)-1
  };

  BlockScope(const std::string &name, const std::string &docComment,
             StatementPtr stmt, KindOf kind);
  virtual ~BlockScope() {}
  bool is(KindOf kind) const { return kind == m_kind;}
  const std::string &getName() const { return m_name;}
  void setName(const std::string name) { m_name = name;}
  virtual std::string getId(CodeGenerator &cg) const;
  StatementPtr getStmt() { return m_stmt;}
  VariableTablePtr getVariables() { return m_variables;}
  ConstantTablePtr getConstants() { return m_constants;}
  ClassScopePtr getContainingClass();
  FunctionScopePtr getContainingFunction();
  FileScopePtr getContainingFile();

  void addUse(BlockScopePtr user, int useFlags);
  void changed(BlockScopeRawPtrQueue &todo, int useKinds);


  /**
   * Helpers for keeping track of break/continue nested level.
   */
  void incLoopNestedLevel();
  void decLoopNestedLevel();
  int getLoopNestedLevel() const { return m_loopNestedLevel;}

  /**
   * Helpers for conditional includes.
   */
  void setIncludeLevel(int incLevel) { m_incLevel = incLevel;}
  int getIncludeLevel() const { return m_incLevel;}

  /**
   * Helpers for parsing class functions and variables.
   */
  ModifierExpressionPtr setModifiers(ModifierExpressionPtr modifiers);
  ModifierExpressionPtr getModifiers() { return m_modifiers;}

  void setClassInfoAttribute(int flag) {
    m_attributeClassInfo |= flag;
  }
  const std::string &getDocComment() const { return m_docComment;}
  void setDocComment(const std::string &doc) { m_docComment = doc;}

  /**
   * Triggers type inference of all statements inside this block.
   */
  void inferTypes(AnalysisResultPtr ar);

  /**
   * Generate constant and variable declarations.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);

  virtual bool inPseudoMain() {
    return false;
  }

  virtual ClassScopePtr getParentScope(AnalysisResultPtr ar) {
    return ClassScopePtr();
  }

  void setOuterScope(BlockScopePtr o) { m_outerScope = o; }
  BlockScopePtr getOuterScope() { return m_outerScope.lock(); }
  bool isOuterScope() { return m_outerScope.expired(); }
  BlockScopeRawPtrHashSet &getDeps() { return m_deps; }

  void setMark(int m) { m_mark = m; }
  int getMark() const { return m_mark; }
protected:
  std::string m_originalName;
  std::string m_name;
  int m_attributeClassInfo;
  std::string m_docComment;
  StatementPtr m_stmt;
  KindOf m_kind;
  VariableTablePtr m_variables;
  ConstantTablePtr m_constants;
  BlockScopeRawPtr m_outerScope;

  BlockScopeRawPtrHashSet m_deps;
  BlockScopeRawPtrFlagsHashMap m_users;

  int m_loopNestedLevel;
  int m_incLevel;
  ModifierExpressionPtr m_modifiers;
  StatementListPtr m_includes;
  int m_mark;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BLOCK_SCOPE_H__
