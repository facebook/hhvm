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
    : SymbolTable(blockScope), m_emptyJumpTable(false) {
}

///////////////////////////////////////////////////////////////////////////////

TypePtr ConstantTable::add(const std::string &name, TypePtr type,
                           ExpressionPtr exp, AnalysisResultPtr ar,
                           ConstructPtr construct) {

  if (name == "true" || name == "false") {
    return Type::Boolean;
  }

  StringToConstructPtrMap::const_iterator iter = m_values.find(name);
  if (iter == m_values.end()) {
    setType(ar, name, type, true);
    m_declarations[name] = construct;
    m_values[name] = exp;
    return type;
  }

  if (ar->isFirstPass()) {
    if (exp != iter->second) {
      ar->getCodeError()->record(CodeError::DeclaredConstantTwice, construct,
                                 m_declarations[name]);
      m_dynamic.insert(name);
      type = Type::Variant;
    }
    setType(ar, name, type, true);
  }

  return type;
}

void ConstantTable::setDynamic(AnalysisResultPtr ar, const std::string &name) {
  m_dynamic.insert(name);
  setType(ar, name, Type::Variant, true);
}

void ConstantTable::setValue(AnalysisResultPtr ar, const std::string &name,
                             ExpressionPtr value) {
  m_values[name] = value;
}

TypePtr ConstantTable::checkBases(const std::string &name, TypePtr type,
                                  bool coerce, AnalysisResultPtr ar,
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
    if (!super) continue;
    actualType = super->checkConst(name, type, coerce, ar, construct,
                                   super->getBases(), defScope);
    if (defScope) return actualType;
  }
  return actualType;
}

TypePtr ConstantTable::check(const std::string &name, TypePtr type,
                             bool coerce, AnalysisResultPtr ar,
                             ConstructPtr construct,
                             const std::vector<std::string> &bases,
                             BlockScope *&defScope) {
  TypePtr actualType;
  defScope = NULL;
  if (name == "true" || name == "false") {
    actualType = Type::Boolean;
  } else if (m_values.find(name) == m_values.end()) {
    if (ar->getPhase() != AnalysisResult::AnalyzeInclude) {
      actualType = checkBases(name, type, coerce, ar, construct,
                              bases, defScope);
      if (defScope) return actualType;
      ar->getCodeError()->record(CodeError::UseUndeclaredConstant,
                                 construct);
      if (m_blockScope.is(BlockScope::ClassScope)) {
        actualType = Type::Variant;
      } else {
        actualType = Type::String;
      }
      setType(ar, name, actualType, true);
    }
  } else {
    StringToTypePtrMap::const_iterator iter = m_coerced.find(name);
    if (iter != m_coerced.end()) {
      defScope = &m_blockScope;
      actualType = iter->second;
      if (actualType->is(Type::KindOfSome) ||
          actualType->is(Type::KindOfAny)) {
        setType(ar, name, type, true);
        return type;
      }
    } else {
      actualType = checkBases(name, type, coerce, ar, construct,
                              bases, defScope);
      if (defScope) return actualType;
      actualType = NEW_TYPE(Some);
      setType(ar, name, actualType, true);
      m_declarations[name] = construct;
    }
  }

  if (Type::IsBadTypeConversion(ar, actualType, type, coerce)) {
    ar->getCodeError()->record(construct, type->getKindOf(),
                               actualType->getKindOf());
  }
  return actualType;
}

ClassScopePtr ConstantTable::findParent(AnalysisResultPtr ar,
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
    for (unsigned int i = 0; i < m_symbols.size(); i++) {
      const string &name = m_symbols[i];
      if (isSystem(name)) continue;

      cg.printf("// @const  %s\t$%s\n",
                getFinalType(name)->toString().c_str(), name.c_str());
    }
  }
}

void ConstantTable::outputCPPDynamicDecl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  const char *prefix = Option::ConstantPrefix;
  string classId;
  const char *fmt = "Variant %s%s%s;\n";
  ClassScopePtr scope = ar->getClassScope();
  if (scope) {
    prefix = Option::ClassConstantPrefix;
    classId = scope->getId();
    fmt = "Variant %s%s_%s;\n";
  }

  for (StringToConstructPtrMap::const_iterator iter = m_declarations.begin();
       iter != m_declarations.end(); ++iter) {
    const string &name = iter->first;
    if (isDynamic(name)) {
      cg.printf(fmt, prefix, classId.c_str(), name.c_str());
    }
  }
}

void ConstantTable::outputCPPDynamicImpl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  for (StringToConstructPtrMap::const_iterator iter = m_declarations.begin();
       iter != m_declarations.end(); ++iter) {
    const string &name = iter->first;
    if (isDynamic(name)) {
      const char *nameStr = name.c_str();
      cg.printf("%s%s = \"%s\";\n", Option::ConstantPrefix, nameStr, nameStr);
    }
  }
}

void ConstantTable::outputCPPGlobalState(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  for (StringToConstructPtrMap::const_iterator iter = m_declarations.begin();
       iter != m_declarations.end(); ++iter) {
    const string &name = iter->first;
    if (isDynamic(name)) {
      cg.printf("dynamic_constants.set(\"%s%s\", g->%s%s);\n",
                Option::ConstantPrefix, name.c_str(),
                Option::ConstantPrefix, name.c_str());
    }
  }
}

void ConstantTable::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool decl = true;
  if (cg.getContext() == CodeGenerator::CppConstantsDecl) {
    decl = false;
  }

  bool printed = false;
  for (StringToConstructPtrMap::const_iterator iter = m_declarations.begin();
       iter != m_declarations.end(); ++iter) {
    const string &name = iter->first;
    if (isSystem(name) && cg.getOutput() != CodeGenerator::SystemCPP) continue;

    ConstructPtr value = getValue(name);
    if (isDynamic(name)) continue;
    printed = true;

    cg.printf(decl ? "extern const " : "const ");
    TypePtr type = getFinalType(name);
    bool isString = type->is(Type::KindOfString);
    if (isString) {
      cg.printf("StaticString");
    } else {
      type->outputCPPDecl(cg, ar);
    }
    const char *nameStr = name.c_str();
    if (decl) {
      cg.printf(" %s%s", Option::ConstantPrefix, nameStr);
    } else {
      cg.printf(" %s%s", Option::ConstantPrefix, nameStr);
      cg.printf(isString ? "(" : " = ");
      if (value) {
        ExpressionPtr exp = dynamic_pointer_cast<Expression>(value);
        ASSERT(!exp->getExpectedType());
        ScalarExpressionPtr scalarExp =
          dynamic_pointer_cast<ScalarExpression>(exp);
        if (isString && scalarExp) {
          cg.printf("LITSTR_INIT(%s)",
                    scalarExp->getCPPLiteralString().c_str());
        } else {
          exp->outputCPP(cg, ar);
        }
      } else {
        cg.printf("\"%s\"", nameStr);
      }
      if (isString) {
        cg.printf(")");
      }
    }
    cg.printf(";\n");
  }
  if (printed) {
    cg.printf("\n");
  }
}

void ConstantTable::outputCPPJumpTable(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       bool needsGlobals,
                                       bool ret) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool hasDynamic = m_dynamic.size() > 0;
  vector<const char *> strings;
  if (!m_symbols.empty()) {
    strings.reserve(m_symbols.size());
    BOOST_FOREACH(string s, m_symbols) {
      // Extension defined constants have no value but we are sure they exist
      if (!system && !getValue(s)) continue;
      strings.push_back(s.c_str());
    }
  }

  m_emptyJumpTable = strings.empty();
  if (!m_emptyJumpTable) {
    if (hasDynamic) {
      if (needsGlobals) {
        cg.printDeclareGlobals();
      }
      ClassScopePtr cls = ar->getClassScope();
      if (cls && cls->needLazyStaticInitializer()) {
        cg.printf("lazy_initializer(g);\n");
      }
    }
    for (JumpTable jt(cg, strings, false, false, false); jt.ready();
         jt.next()) {
      const char *name = jt.key();
      string varName = string(Option::ClassConstantPrefix) +
        getScope()->getId() + "_" + name;
      if (isDynamic(name)) {
        varName = string("g->") + varName;
      }
      cg.printf("HASH_RETURN(0x%016llXLL, %s, %s);\n",
                hash_string(name), varName.c_str(), name);
    }
  }
  if (ret) {
    // TODO this is wrong
    cg.printf("return s;\n");
  }
}

void ConstantTable::outputCPPConstantSymbol(CodeGenerator &cg,
                                            AnalysisResultPtr ar,
                                            const std::string &name) {
  bool cls = ar->getClassScope();
  StringToConstructPtrMap::const_iterator iter = m_values.find(name);
  if (iter != m_values.end() &&
      (!isDynamic(name) || cls)  &&
      !ar->isConstantRedeclared(name)) {
    ExpressionPtr value = dynamic_pointer_cast<Expression>(iter->second);
    Variant v;
    if (value && value->getScalarValue(v)) {
      int len;
      string output = getEscapedText(v, len);
      cg.printf("\"%s\", (const char *)%d, \"%s\",\n", name.c_str(),
                len, output.c_str());
    } else {
      cg.printf("\"%s\", (const char *)0, NULL,\n", name.c_str());
    }
  }
}

void ConstantTable::outputCPPClassMap(CodeGenerator &cg,
                                      AnalysisResultPtr ar,
                                      bool last /* = true */) {
  for (unsigned int i = 0; i < m_symbols.size(); i++) {
    outputCPPConstantSymbol(cg, ar, m_symbols[i]);
  }
  if (last) cg.printf("NULL,\n");
}
