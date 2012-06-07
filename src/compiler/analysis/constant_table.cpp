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

///////////////////////////////////////////////////////////////////////////////

ConstantTable::ConstantTable(BlockScope &blockScope)
    : SymbolTable(blockScope, true),
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
    ASSERT(!sym->valueSet());
    setType(ar, sym, type, true);
    sym->setDeclaration(construct);
    sym->setValue(exp);
    return type;
  }
  ASSERT(sym->declarationSet() && sym->valueSet());

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
                               const std::string &name, bool forceVariant) {
  Symbol *sym = genSymbol(name, true);
  if (!sym->isDynamic()) {
    Lock lock(BlockScope::s_constMutex);
    sym->setDynamic();
    if (sym->getDeclaration()) {
      sym->getDeclaration()->getScope()->
        addUpdates(BlockScope::UseKindConstRef);
    }
    m_hasDynamic = true;
    if (forceVariant) {
      setType(ar, sym, Type::Variant, true);
    }
  }
}

void ConstantTable::setValue(AnalysisResultConstPtr ar, const std::string &name,
                             ExpressionPtr value) {
  Symbol *sym = getSymbol(name);
  ASSERT(sym && sym->isPresent());
  sym->setValue(value);
}

bool ConstantTable::isRecursivelyDeclared(AnalysisResultConstPtr ar,
                                          const std::string &name) const {
  if (const Symbol *sym ATTRIBUTE_UNUSED = getSymbol(name)) {
    ASSERT(sym->isPresent() && sym->valueSet());
    return true;
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    return parent->getConstants()->isRecursivelyDeclared(ar, name);
  }
  return false;
}

ConstructPtr ConstantTable::getValueRecur(AnalysisResultConstPtr ar,
                                          const std::string &name,
                                          ClassScopePtr &defClass) const {
  if (const Symbol *sym = getSymbol(name)) {
    ASSERT(sym->isPresent() && sym->valueSet());
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
                                                ClassScopePtr &defClass)
const {
  if (const Symbol *sym = getSymbol(name)) {
    ASSERT(sym->isPresent() && sym->valueSet());
    if (sym->getDeclaration()) return sym->getDeclaration();
  }
  ClassScopePtr parent = findParent(ar, name);
  if (parent) {
    defClass = parent;
    return parent->getConstants()->getDeclarationRecur(ar, name, defClass);
  }
  return ConstructPtr();
}

void ConstantTable::cleanupForError(AnalysisResultConstPtr ar) {
  AnalysisResult::Locker lock(ar);

  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (!sym->isDynamic()) {
      sym->setDynamic();
      sym->setDeclaration(ConstructPtr());
      sym->setValue(ConstructPtr());
    }
  }
}

TypePtr ConstantTable::check(BlockScopeRawPtr context,
                             const std::string &name, TypePtr type,
                             bool coerce, AnalysisResultConstPtr ar,
                             ConstructPtr construct,
                             const std::vector<std::string> &bases,
                             BlockScope *&defScope) {
  ASSERT(!m_blockScope.is(BlockScope::FunctionScope));
  bool isClassScope = m_blockScope.is(BlockScope::ClassScope);
  TypePtr actualType;
  defScope = NULL;
  if (name == "true" || name == "false") {
    actualType = Type::Boolean;
  } else {
    Symbol *sym = getSymbol(name);
    if (!sym) {
      if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
        if (isClassScope) {
          ClassScopeRawPtr parent = findBase(ar, name, bases);
          if (parent) {
            actualType = parent->getConstants()->check(
              context, name, type, coerce, ar, construct, bases, defScope);
            if (defScope) return actualType;
          }
        }
        Compiler::Error(Compiler::UseUndeclaredConstant, construct);
        actualType = isClassScope || !Option::WholeProgram ?
          Type::Variant : Type::String;
      }
    } else {
      ASSERT(sym->isPresent());
      ASSERT(sym->getType());
      ASSERT(sym->isConstant());
      defScope = &m_blockScope;
      if (isClassScope) {
        // if the current scope is a function scope, grab the lock.
        // otherwise if it's a class scope, then *try* to grab the lock.
        if (context->is(BlockScope::FunctionScope)) {
          GET_LOCK(BlockScopeRawPtr(&m_blockScope));
          return setType(ar, sym, type, coerce);
        } else {
          TRY_LOCK(BlockScopeRawPtr(&m_blockScope));
          return setType(ar, sym, type, coerce);
        }
      } else {
        Lock lock(m_blockScope.getMutex());
        return setType(ar, sym, type, coerce);
      }
    }
  }

  return actualType;
}

ClassScopePtr ConstantTable::findParent(AnalysisResultConstPtr ar,
                                        const std::string &name) const {
  for (ClassScopePtr parent = m_blockScope.getParentScope(ar);
       parent && !parent->isRedeclaring();
       parent = parent->getParentScope(ar)) {
    if (parent->hasConst(name)) {
      return parent;
    }
  }
  return ClassScopePtr();
}

ClassScopeRawPtr ConstantTable::findBase(
  AnalysisResultConstPtr ar, const std::string &name,
  const std::vector<std::string> &bases) const {
  for (int i = bases.size(); i--; ) {
    ClassScopeRawPtr p = ar->findClass(bases[i]);
    if (!p || p->isRedeclaring()) continue;
    if (p->hasConst(name)) return p;
    ConstantTablePtr constants = p->getConstants();
    p = constants->findBase(ar, name, p->getBases());
    if (p) return p;
  }
  return ClassScopeRawPtr();
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

void ConstantTable::getCPPDynamicDecl(CodeGenerator &cg,
                                      AnalysisResultPtr ar,
                                      Type2SymbolSetMap &type2names) {
  const char *prefix = Option::ConstantPrefix;
  string classId;
  const char *fmt = "";
  ClassScopePtr scope = getClassScope();
  if (scope) {
    prefix = Option::ClassConstantPrefix;
    classId = scope->getId();
    fmt = Option::IdPrefix.c_str();
  }

  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

  SymbolSet &symbols = type2names["Variant"];
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (sym->declarationSet() && sym->isDynamic() &&
        system == sym->isSystem()) {
      string tmp = string(prefix) + classId + fmt +
        CodeGenerator::FormatLabel(sym->getName());
      if (Type::IsMappedToVariant(sym->getFinalType())) {
        symbols.insert(tmp);
      } else {
        type2names[sym->getFinalType()->getCPPDecl(ar, BlockScopeRawPtr())].
          insert(tmp);
      }
    }
  }
}

void ConstantTable::outputCPPDynamicImpl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (sym->declarationSet() && sym->isDynamic()) {
      cg_printf("%s%s = \"%s\";\n", Option::ConstantPrefix,
                CodeGenerator::FormatLabel(sym->getName()).c_str(),
                CodeGenerator::EscapeLabel(sym->getName()).c_str());
    }
  }
}

void ConstantTable::collectCPPGlobalSymbols(StringPairSet &symbols,
                                            CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  BOOST_FOREACH(Symbol *sym, m_symbolVec) {
    if (sym->declarationSet() && sym->isDynamic()) {
      string varname = Option::ConstantPrefix +
                       CodeGenerator::FormatLabel(sym->getName());
      symbols.insert(StringPair(varname, varname));
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
                CodeGenerator::FormatLabel(name).c_str());
    } else {
      cg_printf(" %s%s%s%s", Option::ClassConstantPrefix,
                cls->getId().c_str(),
                Option::IdPrefix.c_str(),
                CodeGenerator::FormatLabel(name).c_str());
    }
  } else {
    if (!cls) {
      cg_printf(" %s%s", Option::ConstantPrefix,
                CodeGenerator::FormatLabel(name).c_str());
    } else {
      cg_printf(" %s%s%s%s", Option::ClassConstantPrefix,
                cls->getId().c_str(),
                Option::IdPrefix.c_str(),
                CodeGenerator::FormatLabel(name).c_str());
    }
    cg_printf(isString ? "(" : " = ");
    if (value) {
      ExpressionPtr exp = dynamic_pointer_cast<Expression>(value);
      if (isString && exp->isScalar()) {
        ScalarExpressionPtr scalarExp =
          dynamic_pointer_cast<ScalarExpression>(exp);
        if (scalarExp) {
          cg_printf("LITSTR_INIT(%s)",
                    scalarExp->getCPPLiteralString().c_str());
        } else {
          Variant v;
          exp->getScalarValue(v);
          cg_printf("LITSTR_INIT(\"%s\")",
                    CodeGenerator::EscapeLabel(v.toString().data()).c_str());
        }
      } else {
        exp->outputCPP(cg, ar);
      }
    } else {
      cg_printf("\"%s\"", CodeGenerator::EscapeLabel(name).c_str());
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

void ConstantTable::outputCPPConstantSymbol(CodeGenerator &cg,
                                            AnalysisResultPtr ar,
                                            Symbol *sym) {
  ClassScopeRawPtr cls = getClassScope();
  if (sym->valueSet() &&
      (cls || (!sym->isDynamic()  &&
               !ar->isConstantRedeclared(sym->getName())))) {
    ExpressionPtr value = dynamic_pointer_cast<Expression>(sym->getValue());
    Variant v;
    if (value && value->getScalarValue(v)) {
      int len;
      string output = getEscapedText(v, len);
      cg_printf("\"%s\", (const char *)%d, \"%s\",\n",
                CodeGenerator::EscapeLabel(sym->getName()).c_str(),
                len, output.c_str());
    } else if (cls) {
      cg_printf("\"%s\", (const char *)&%s%s, NULL,\n",
                CodeGenerator::EscapeLabel(sym->getName()).c_str(),
                Option::ClassStaticsCallbackPrefix, cls->getId().c_str());
    } else {
      cg_printf("\"%s\", (const char *)0, NULL,\n",
                CodeGenerator::EscapeLabel(sym->getName()).c_str());
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
