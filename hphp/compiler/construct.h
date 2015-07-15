/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CONSTRUCT_H_
#define incl_HPHP_CONSTRUCT_H_

#include "hphp/parser/location.h"
#include "hphp/compiler/json.h"
#include <memory>
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/block_scope.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Variant;
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(IParseHandler);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FileScope);

class AstWalkerStateVec;

class IParseHandler {
  /**
   * To avoid iteration of parse tree, we move any work that can be done
   * in parse phase into this function, so to speed up static analysis.
   */
public:
  virtual ~IParseHandler() {}

  /**
   * onParse is called by the parser when the construct has just been parsed
   * to allow it to do any necessary work
   */
  virtual void onParse(AnalysisResultConstPtr ar, FileScopePtr scope) {
    always_assert(0);
  }
  /**
   * onParseRecur is called by a parent construct (ultimately a class or
   * interface).
   * This is done because at the time that onParse would be called for
   * (eg) a method, the ClassScope doesnt exist. So we wait until onParse
   * is called for the class, and it calls onParseRecur for its children.
   */
  virtual void onParseRecur(AnalysisResultConstPtr ar, FileScopeRawPtr fs,
                            ClassScopePtr scope) {
    always_assert(0);
  }
};

#define DECLARE_STATEMENT_TYPES(x) \
  x(Statement)              \
  x(FunctionStatement)      \
  x(ClassStatement)         \
  x(InterfaceStatement)     \
  x(ClassVariable)          \
  x(ClassConstant)          \
  x(MethodStatement)        \
  x(StatementList)          \
  x(BlockStatement)         \
  x(IfBranchStatement)      \
  x(IfStatement)            \
  x(WhileStatement)         \
  x(DoStatement)            \
  x(ForStatement)           \
  x(SwitchStatement)        \
  x(CaseStatement)          \
  x(BreakStatement)         \
  x(ContinueStatement)      \
  x(ReturnStatement)        \
  x(GlobalStatement)        \
  x(StaticStatement)        \
  x(EchoStatement)          \
  x(UnsetStatement)         \
  x(ExpStatement)           \
  x(ForEachStatement)       \
  x(FinallyStatement)       \
  x(CatchStatement)         \
  x(TryStatement)           \
  x(ThrowStatement)         \
  x(GotoStatement)          \
  x(LabelStatement)         \
  x(UseTraitStatement)      \
  x(ClassRequireStatement)  \
  x(TraitPrecStatement)     \
  x(TraitAliasStatement)    \
  x(TypedefStatement)

#define DECLARE_EXPRESSION_TYPES(x)     \
  x(Expression,                  None) \
  x(ExpressionList,              None) \
  x(AssignmentExpression,       Store) \
  x(SimpleVariable,              Load) \
  x(DynamicVariable,             Load) \
  x(StaticMemberExpression,      Load) \
  x(ArrayElementExpression,      Load) \
  x(DynamicFunctionCall,         Call) \
  x(SimpleFunctionCall,          Call) \
  x(ScalarExpression,            None) \
  x(ObjectPropertyExpression,    Load) \
  x(ObjectMethodExpression,      Call) \
  x(ListAssignment,             Store) \
  x(NewObjectExpression,         Call) \
  x(UnaryOpExpression,         Update) \
  x(IncludeExpression,           Call) \
  x(BinaryOpExpression,        Update) \
  x(QOpExpression,               None) \
  x(ArrayPairExpression,         None) \
  x(ClassConstantExpression,    Const) \
  x(ParameterExpression,         None) \
  x(ModifierExpression,          None) \
  x(ConstantExpression,         Const) \
  x(EncapsListExpression,        None) \
  x(ClosureExpression,           None) \
  x(YieldExpression,             None) \
  x(AwaitExpression,             None) \
  x(UserAttribute,               None) \
  x(QueryExpression,             None) \
  x(FromClause,                  None) \
  x(LetClause,                   None) \
  x(WhereClause,                 None) \
  x(SelectClause,                None) \
  x(IntoClause,                  None) \
  x(JoinClause,                  None) \
  x(GroupClause,                 None) \
  x(OrderbyClause,               None) \
  x(Ordering,                    None)

/**
 * Base class of Expression and Statement.
 */
class Construct : public std::enable_shared_from_this<Construct>,
                  public JSON::CodeError::ISerializable {
public:
  virtual ~Construct() {}

#define DEC_STATEMENT_ENUM(x) KindOf##x,
#define DEC_EXPRESSION_ENUM(x,t) KindOf##x,
  enum KindOf {
    DECLARE_STATEMENT_TYPES(DEC_STATEMENT_ENUM)
    DECLARE_EXPRESSION_TYPES(DEC_EXPRESSION_ENUM)
  };
#undef DEC_EXPRESSION_ENUM
#undef DEC_STATEMENT_ENUM

protected:
  Construct(BlockScopePtr scope, const Location::Range& loc, KindOf);

public:
  /**
   * Type checking without RTTI.
   */
  bool is(KindOf type) const {
    assert(m_kindOf != KindOfStatement);
    assert(m_kindOf != KindOfExpression);
    return m_kindOf == type;
  }
  KindOf getKindOf() const {
    assert(m_kindOf != KindOfStatement);
    assert(m_kindOf != KindOfExpression);
    return m_kindOf;
  }

  bool isStatement() const {
    return !isExpression();
  }
  bool isExpression() const {
    return m_kindOf > KindOfExpression;
  }

  enum Effect {
    NoEffect = 0,
    IOEffect = 1,                // could have an observable effect (not
                                 // changing variable values)
    AssignEffect = 2,            // writes an object in a way understood by the
                                 // alias manager
    GlobalEffect = 4,            // could affect global variables
    LocalEffect = 8,             // could affect variables from the local scope
    ParamEffect = 0x10,          // a function could affect its parameters
    DeepParamEffect = 0x20,      // a function could affect the array elements
                                 // or object members referenced by its
                                 // parameters
    DynamicParamEffect = 0x40,   // a function creates dynamic exps based
                                 // on its parameters, which it could affect
    CanThrow = 0x80,             // can throw PHP exception
    AccessorEffect = 0x100,      // could contain a getter/setter
    CreateEffect = 0x200,        // could cause the creation of an array
                                 // element or an object property
    DiagnosticEffect = 0x400,    // can cause a diagnostic to be issued
    OtherEffect = 0x800,         // something else
    UnknownEffect = 0xfff        // any of the above
  };

  void copyLocationTo(ConstructPtr other);
  const Location::Range& getRange() const { return m_r; }
  int line0() const { return m_r.line0; }
  int line1() const { return m_r.line1; }
  void setFirst(int line0, int char0) { m_r.line0 = line0; m_r.char0 = char0; }
  void setFileLevel() { m_flags.topLevel = m_flags.fileLevel = true;}
  void setTopLevel() { m_flags.topLevel = true;}
  void setVisited() { m_flags.visited = true;}
  void clearVisited() { m_flags.visited = false;}
  bool isFileLevel() const { return m_flags.fileLevel;}
  bool isTopLevel() const { return m_flags.topLevel;}
  bool isVisited() const { return m_flags.visited; }

  void setAnticipated() { m_flags.anticipated = true; }
  void clearAnticipated() { m_flags.anticipated = false; }
  bool isAnticipated() const { return m_flags.anticipated; }

  void setAvailable() { m_flags.available = true; }
  void clearAvailable() { m_flags.available = false; }
  bool isAvailable() const { return m_flags.available; }

  void setNonNull() { m_flags.nonNull = true; }
  void clearNonNull() { m_flags.nonNull = false; }
  bool isNonNull() const { return m_flags.nonNull; }

  void setLocalExprAltered() { m_flags.localExprNotAltered = false; }
  void clearLocalExprAltered() { m_flags.localExprNotAltered = true; }
  bool isLocalExprAltered() const { return !m_flags.localExprNotAltered; }

  void setReferencedValid() { m_flags.referenced_valid = true; }
  void clearReferencedValid() { m_flags.referenced_valid = false; }
  bool isReferencedValid() const { return m_flags.referenced_valid; }

  void setReferenced() { m_flags.referenced = true; }
  void clearReferenced() { m_flags.referenced = false; }
  bool isReferenced() const { return m_flags.referenced; }

  void setNeeded() { m_flags.needed = true; }
  void clearNeeded() { m_flags.needed = false; }
  bool isNeeded() const { return m_flags.needed; }

  void setNoRemove() { m_flags.noRemove = true; }
  void clearNoRemove() { m_flags.noRemove = false; }
  bool isNoRemove() const { return m_flags.noRemove; }

  void setGuarded() { m_flags.guarded = true; }
  void clearGuarded() { m_flags.guarded = false; }
  bool isGuarded() const { return m_flags.guarded; }

  void setRefCounted() { m_flags.refCounted = 3; }
  void clearRefCounted() { m_flags.refCounted = 2; }
  bool maybeRefCounted() const {
    return !(m_flags.refCounted & 2) || (m_flags.refCounted & 1);
  }

  void setInited() { m_flags.inited = 3; }
  void clearInited() { m_flags.inited = 2; }
  bool maybeInited() const {
    return !(m_flags.inited & 2) || (m_flags.inited & 1);
  }
  void setIsUnpack() { m_flags.unpack = 1; }
  bool isUnpack() const { return m_flags.unpack; }
  void clearIsUnpack() { m_flags.unpack = 0; }

  void setKilled() { m_flags.killed = true; }
  void clearKilled() { m_flags.killed = false; }
  bool isKilled() const { return m_flags.killed; }

  BlockScopeRawPtr getScope() const { return m_blockScope; }
  void setBlockScope(BlockScopeRawPtr scope) { m_blockScope = scope; }
  FileScopeRawPtr getFileScope() const {
    return m_blockScope->getContainingFile();
  }
  FunctionScopeRawPtr getFunctionScope() const {
    return m_blockScope->getContainingFunction();
  }
  ClassScopeRawPtr getClassScope() const {
    return m_blockScope->getContainingClass();
  }
  void resetScope(BlockScopeRawPtr scope);
  void parseTimeFatal(FileScopeRawPtr fs, Compiler::ErrorType error,
                      const char *fmt, ...) ATTRIBUTE_PRINTF(4,5);
  void analysisTimeFatal(Compiler::ErrorType error, const char *fmt, ...)
    ATTRIBUTE_PRINTF(3,4);
  virtual int getLocalEffects() const { return UnknownEffect;}
  int getChildrenEffects() const;
  int getContainedEffects() const;
  bool hasEffect() const { return getContainedEffects() != NoEffect;}
  virtual bool kidUnused(int i) const { return false; }

  template<typename T>
  static std::shared_ptr<T> Clone(std::shared_ptr<T> constr) {
    if (constr) {
      return dynamic_pointer_cast<T>(constr->clone());
    }
    return std::shared_ptr<T>();
  }

  template<typename T>
  std::shared_ptr<T> Clone(std::shared_ptr<T> constr,
                           BlockScopePtr scope) {
    if (constr) {
      constr = constr->clone();
      constr->resetScope(scope);
    }
    return constr;
  }

  /**
   * Called when we analyze a program, which file it includes, which function
   * and class it uses, etc.
   */
  virtual void analyzeProgram(AnalysisResultPtr ar) = 0;

  /**
   * return the nth child construct
   */
  virtual ConstructPtr getNthKid(int n) const { return ConstructPtr(); }

  /**
   * set the nth child construct
   */
  virtual void setNthKid(int n, ConstructPtr cp)  {}

  /**
   * get the kid count
   */
  virtual int getKidCount() const = 0;

  // helpers for GDB
  void dumpNode(int spc);
  void dumpNode(int spc) const;

  void dump(int spc, AnalysisResultConstPtr ar);
  void dumpNode(int spc, AnalysisResultConstPtr ar);

  static void dump(int spc, AnalysisResultConstPtr ar, bool functionOnly,
                   const AstWalkerStateVec &start,
                   ConstructPtr endBefore, ConstructPtr endAfter);

  /**
   * Generates a serialized Code Model corresponding to this AST.
   */
  virtual void outputCodeModel(CodeGenerator &cg) = 0;

  /**
   * Called when generating code.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) = 0;

  /**
   * Implements JSON::CodeError::ISerializable.
   */
  virtual void serialize(JSON::CodeError::OutputStream &out) const;

  /**
   * Get canonicalized PHP source code for this construct.
   */
  std::string getText(AnalysisResultPtr ar = AnalysisResultPtr());

  void recomputeEffects();

  /**
   * Write where this construct was in PHP files.
   */
  void printSource(CodeGenerator &cg);
  ExpressionPtr makeConstant(AnalysisResultConstPtr ar,
                             const std::string &value) const;
  ExpressionPtr makeScalarExpression(AnalysisResultConstPtr ar,
                                     const Variant &value) const;
private:
  BlockScopeRawPtr m_blockScope;
  union {
    unsigned m_flagsVal;
    struct {
      unsigned fileLevel           : 1; // is it at top level of a file
      unsigned topLevel            : 1; // is it at top level of a scope
      unsigned visited             : 1; // general purpose for walks
      unsigned anticipated         : 1;
      unsigned available           : 1;
      unsigned localExprNotAltered : 1; // whether this node can be
                                        // altered in this expression
      unsigned nonNull             : 1; // expression is not null
      unsigned referenced          : 1;
      unsigned referenced_valid    : 1; // is the above flag is valid
      unsigned needed              : 1;
      unsigned noRemove            : 1; // DCE should NOT remove this node
      unsigned guarded             : 1; // previously used
      unsigned killed              : 1;
      unsigned refCounted          : 2; // high bit indicates whether its valid
      unsigned inited              : 2; // high bit indicates whether its valid
      unsigned unpack              : 1; // is this an unpack (only on params)
    } m_flags;
  };
  Location::Range m_r;
protected:
  KindOf m_kindOf;
  mutable int m_containedEffects;
  mutable int m_effectsTag;
};

class LocalEffectsContainer {
public:
  int getLocalEffects() const { return m_localEffects; }
  virtual void effectsCallback() = 0;
protected:
  explicit LocalEffectsContainer(Construct::Effect localEffect) :
    m_localEffects(localEffect) {}
  LocalEffectsContainer() :
    m_localEffects(0) {}
  void setLocalEffect  (Construct::Effect effect);
  void clearLocalEffect(Construct::Effect effect);
  bool hasLocalEffect  (Construct::Effect effect) const;
protected:
  int m_localEffects;
};

#define DECL_AND_IMPL_LOCAL_EFFECTS_METHODS \
  int getLocalEffects() const override { \
    return LocalEffectsContainer::getLocalEffects(); \
  } \
  void effectsCallback() override { recomputeEffects(); }

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CONSTRUCT_H_
