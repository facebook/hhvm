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

#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/type.h>
#include <compiler/code_generator.h>
#include <compiler/expression/expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/option.h>
#include <util/util.h>
#include <util/hash.h>
#include <compiler/analysis/class_scope.h>
#include <runtime/base/complex_types.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

ConstantTable::ConstantTable(BlockScope &blockScope)
    : SymbolTable(blockScope, true), m_emptyJumpTable(false),
      m_hasDynamic(false) {
}

///////////////////////////////////////////////////////////////////////////////

TypePtr ConstantTable::add(const std::string &name, TypePtr type,
                           ExpressionPtr exp, AnalysisResultConstPtr ar,
                           ConstructPtr construct) {

  if (name == "true" || name == "false") {
    return Type::Boolean;
  }

  Symbol *sym = genSymbol(name, true);
  if (!sym->declarationSet()) {
    setType(ar, sym, type, true);
    sym->setDeclaration(construct);
    sym->setValue(exp);
    return type;
  }

  if (m_blockScope.isFirstPass()) {
    if (construct) {
      if (exp != sym->getValue()) {
        Compiler::Error(Compiler::DeclaredConstantTwice, construct,
                        sym->getDeclaration());
        if (!sym->isDynamic()) {
          sym->setDynamic();
          m_hasDynamic = true;
        }
        type = Type::Variant;
      }
    } else if (exp) {
      sym->setValue(exp);
    }
    setType(ar, sym, type, true);
  }

  return type;
}

void ConstantTable::setDynamic(AnalysisResultConstPtr ar,
                               const std::string &name) {
  Symbol *sym = genSymbol(name, true);
  if (!sym->isDynamic()) {
    Lock lock(BlockScope::s_constMutex);
    sym->setDynamic();
    if (sym->getDeclaration()) {
      sym->getDeclaration()->getScope()->
        addUpdates(BlockScope::UseKindConstRef);
    }
    m_hasDynamic = true;
    setType(ar, sym, Type::Variant, true);
  }
}

void ConstantTable::setValue(AnalysisResultConstPtr ar, const std::string &name,
                             ExpressionPtr value) {
  genSymbol(name, true)->setValue(value);
}

bool ConstantTable::isRecursivelyDeclared(AnalysisResultConstPtr ar,
                                          const std::string &name) {
  if (Symbol *sym = getSymbol(name)) {
    if (sym->valueSet()) return true;
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    return parent->getConstants()->isRecursivelyDeclared(ar, name);
  }
  return false;
}

ConstructPtr ConstantTable::getValueRecur(AnalysisResultConstPtr ar,
                                          const std::string &name,
                                          ClassScopePtr &defClass) {
  if (Symbol *sym = getSymbol(name)) {
    if (sym->getValue()) return sym->getValue();
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    defClass = parent;
    return parent->getConstants()->getValueRecur(ar, name, defClass);
  }
  return ConstructPtr();
}

ConstructPtr ConstantTable::getDeclarationRecur(AnalysisResultConstPtr ar,
                                                const std::string &name,
                                                ClassScopePtr &defClass) {
  if (Symbol *sym = getSymbol(name)) {
    if (sym->getDeclaration()) return sym->getDeclaration();
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    defClass = parent;
    return parent->getConstants()->getDeclarationRecur(ar, name, defClass);
  }
  return ConstructPtr();
}

TypePtr ConstantTable::checkBases(const std::string &name, TypePtr type,
                                  bool coerce, AnalysisResultConstPtr ar,
                                  ConstructPtr construct,
                                  const std::vector<std::string> &bases,
                                  BlockScope *&defScope) {
  TypePtr actualType;
  defScope = NULL;
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    actualType = parent->checkConst(name, type, coerce, ar, construct,
                                    parent->getBases(), defScope);
    if (defScope) return actualType;
  }
  for (int i = bases.size() - 1; i >= (parent ? 1 : 0); i--) {
    const string &base = bases[i];
    ClassScopePtr super = ar->findClass(base);
    if (!super || super->isRedeclaring()) continue;
    actualType = super->checkConst(name, type, coerce, ar, construct,
                                   super->getBases(), defScope);
    if (defScope) return actualType;
  }
  return actualType;
}

TypePtr ConstantTable::check(const std::string &name, TypePtr type,
                             bool coerce, AnalysisResultConstPtr ar,
                             ConstructPtr construct,
                             const std::vector<std::string> &bases,
                             BlockScope *&defScope) {
  TypePtr actualType;
  defScope = NULL;
  if (name == "true" || name == "false") {
    actualType = Type::Boolean;
  } else {
    Symbol *sym = genSymbol(name, true);
    if (!sym->valueSet()) {
      if (ar->getPhase() != AnalysisResult::AnalyzeInclude) {
        actualType = checkBases(name, type, coerce, ar, construct,
                                bases, defScope);
        if (defScope) return actualType;
        Compiler::Error(Compiler::UseUndeclaredConstant, construct);
        if (m_blockScope.is(BlockScope::ClassScope)) {
          actualType = Type::Variant;
        } else {
          actualType = Type::String;
        }
        setType(ar, sym, actualType, true);
      }
    } else {
      if (sym->getType()) {
        defScope = &m_blockScope;
        actualType = sym->getType();
        if (actualType->is(Type::KindOfSome) ||
            actualType->is(Type::KindOfAny)) {
          setType(ar, sym, type, true);
          return type;
        }
      } else {
        actualType = checkBases(name, type, coerce, ar, construct,
                                bases, defScope);
        if (defScope) return actualType;
        actualType = Type::Some;
        setType(ar, sym, actualType, true);
        sym->setDeclaration(construct);
      }
    }
  }

  return actualType;
}

ClassScopePtr ConstantTable::findParent(AnalysisResultConstPtr ar,
                                        const std::string &name) {
  for (ClassScopePtr parent = m_blockScope.getParentScope(ar);
       parent && !parent->isRedeclaring();
       parent = parent->getParentScope(ar)) {
    if (parent->hasConst(name)) {
      return parent;
    }
  }
  return ClassScopePtr();
}

///////////////////////////////////////////////////////////////////////////////

void ConstantTable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes) {
    for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
      Symbol *sym = m_symbolVec[i];
      if (sym->isSystem()) continue;

      cg_printf("// @const  %s\t$%s\n",
                sym->getFinalType()->toString().c_str(),
                sym->getName().c_str());
    }
  }
}

void ConstantTable::outputCPPDynamicDecl(CodeGenerator &cg,
                                         AnalysisResultPtr ar,
                                         Type2SymbolListMap &type2names) {
  const char *prefix = Option::ConstantPrefix;
  string classId;
  const char *fmt = "";
  ClassScopePtr scope = getClassScope();
  if (scope) {
    prefix = Option::ClassConstantPrefix;
    classId = scope->getId(cg);
    fmt = "_";
  }

  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

  SymbolList &symbols = type2names["Variant"];
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (sym->declarationSet() && sym->isDynamic() &&
        system == sym->isSystem()) {
      symbols.push_back(string(prefix) + classId + fmt +
                        cg.formatLabel(sym->getName()));
    }
  }
}

void ConstantTable::outputCPPDynamicImpl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (sym->declarationSet() && sym->isDynamic()) {
      cg_printf("%s%s = \"%s\";\n", Option::ConstantPrefix,
                cg.formatLabel(sym->getName()).c_str(),
                cg.escapeLabel(sym->getName()).c_str());
    }
  }
}

void ConstantTable::collectCPPGlobalSymbols(StringPairVec &symbols,
                                            CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (sym->declarationSet() && sym->isDynamic()) {
      string varname = Option::ConstantPrefix + cg.formatLabel(sym->getName());
      symbols.push_back(pair<string, string>(varname, varname));
    }
  }
}

void ConstantTable::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                              bool newline /* = true */) const {
  bool printed = false;
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (outputCPP(cg, ar, sym)) printed = true;
  }
  if (newline && printed) {
    cg_printf("\n");
  }
}

bool ConstantTable::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                              const Symbol *sym) const {
  bool decl = true;
  if (cg.getContext() == CodeGenerator::CppConstantsDecl) {
    decl = false;
  }

  if (!const_cast<Symbol*>(sym)->checkDefined() ||
      sym->isDynamic()) {
    return false;
  }
  if (sym->isSystem() && cg.getOutput() != CodeGenerator::SystemCPP) {
    return false;
  }
  const string &name = sym->getName();
  ConstructPtr value = sym->getValue();

  cg_printf(decl ? "extern const " : "const ");
  TypePtr type = sym->getFinalType();
  bool isString = type->is(Type::KindOfString);
  if (isString) {
    cg_printf("StaticString");
  } else {
    type->outputCPPDecl(cg, ar, getBlockScope());
  }
  ClassScope *cls = dynamic_cast<ClassScope*>(&m_blockScope);
  if (decl) {
    if (!cls) {
      cg_printf(" %s%s", Option::ConstantPrefix,
                cg.formatLabel(name).c_str());
    } else {
      cg_printf(" %s%s_%s", Option::ClassConstantPrefix,
                cls->getId(cg).c_str(),
                cg.formatLabel(name).c_str());
    }
  } else {
    if (!cls) {
      cg_printf(" %s%s", Option::ConstantPrefix,
                cg.formatLabel(name).c_str());
    } else {
      cg_printf(" %s%s_%s", Option::ClassConstantPrefix,
                cls->getId(cg).c_str(),
                cg.formatLabel(name).c_str());
    }
    cg_printf(isString ? "(" : " = ");
    if (value) {
      ExpressionPtr exp = dynamic_pointer_cast<Expression>(value);
      ASSERT(!exp->getExpectedType());
      if (isString && exp->isScalar()) {
        ScalarExpressionPtr scalarExp =
          dynamic_pointer_cast<ScalarExpression>(exp);
        if (scalarExp) {
          cg_printf("LITSTR_INIT(%s)",
                    scalarExp->getCPPLiteralString(cg).c_str());
        } else {
          Variant v;
          exp->getScalarValue(v);
          cg_printf("LITSTR_INIT(\"%s\")",
                    cg.escapeLabel(v.toString().data()).c_str());
        }
      } else {
        exp->outputCPP(cg, ar);
      }
    } else {
      cg_printf("\"%s\"", cg.escapeLabel(name).c_str());
    }
    if (isString) {
      cg_printf(")");
    }
  }
  cg_printf(";\n");
  return true;
}

bool ConstantTable::outputSingleConstant(CodeGenerator &cg,
                                         AnalysisResultPtr ar,
                                         const std::string &name) const {
  const Symbol *sym = getSymbol(name);
  return sym && outputCPP(cg, ar, sym);
}

void ConstantTable::outputCPPJumpTable(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       bool needsGlobals,
                                       bool ret) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  vector<const char *> strings;
  if (!m_symbolVec.empty()) {
    strings.reserve(m_symbolVec.size());
    BOOST_FOREACH(Symbol *sym, m_symbolVec) {
      // Extension defined constants have no value but we are sure they exist
      if (!system && !sym->getValue()) continue;
      strings.push_back(sym->getName().c_str());
    }
  }

  m_emptyJumpTable = strings.empty();
  if (!m_emptyJumpTable) {
    if (m_hasDynamic) {
      if (needsGlobals) {
        cg.printDeclareGlobals();
      }
      ClassScopePtr cls = getClassScope();
      if (cls && cls->needLazyStaticInitializer()) {
        cg_printf("lazy_initializer(g);\n");
      }
    }
    for (JumpTable jt(cg, strings, false, false, false); jt.ready();
         jt.next()) {
      const char *name = jt.key();
      string varName = string(Option::ClassConstantPrefix) +
        getScopePtr()->getId(cg) + "_" + cg.formatLabel(name);
      if (isDynamic(name)) {
        varName = string("g->") + varName;
      }
      cg_printf("HASH_RETURN(0x%016llXLL, %s, \"%s\");\n",
                hash_string(name), varName.c_str(),
                cg.escapeLabel(name).c_str());
    }
  }
  if (ret) {
    // TODO this is wrong
    cg_printf("return s;\n");
  }
}

void ConstantTable::outputCPPConstantSymbol(CodeGenerator &cg,
                                            AnalysisResultPtr ar,
                                            Symbol *sym) {
  bool cls = getClassScope();
  if (sym->valueSet() &&
      (!sym->isDynamic() || cls)  &&
      !ar->isConstantRedeclared(sym->getName())) {
    ExpressionPtr value = dynamic_pointer_cast<Expression>(sym->getValue());
    Variant v;
    if (value && value->getScalarValue(v)) {
      int len;
      string output = getEscapedText(v, len);
      cg_printf("\"%s\", (const char *)%d, \"%s\",\n",
                cg.escapeLabel(sym->getName()).c_str(), len, output.c_str());
    } else {
      cg_printf("\"%s\", (const char *)0, NULL,\n",
                cg.escapeLabel(sym->getName()).c_str());
    }
  }
}

void ConstantTable::outputCPPClassMap(CodeGenerator &cg,
                                      AnalysisResultPtr ar,
                                      bool last /* = true */) {
  for (unsigned int i = 0; i < m_symbolVec.size(); i++) {
    outputCPPConstantSymbol(cg, ar, m_symbolVec[i]);
  }
  if (last) cg_printf("NULL,\n");
}
