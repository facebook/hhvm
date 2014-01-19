/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/type_annotation.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/util/util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/constant_expression.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ParameterExpression::ParameterExpression(
     EXPRESSION_CONSTRUCTOR_PARAMETERS,
     TypeAnnotationPtr type,
     bool hhType,
     const std::string &name,
     bool ref,
     TokenID modifier,
     ExpressionPtr defaultValue,
     ExpressionPtr attributeList)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ParameterExpression))
  , m_originalType(type)
  , m_name(name)
  , m_hhType(hhType)
  , m_ref(ref)
  , m_modifier(modifier)
  , m_defaultValue(defaultValue)
  , m_attributeList(attributeList)
{
  m_type = Util::toLower(type ? type->vanillaName() : "");
  if (m_defaultValue) {
    m_defaultValue->setContext(InParameterExpression);
  }
}

ExpressionPtr ParameterExpression::clone() {
  ParameterExpressionPtr exp(new ParameterExpression(*this));
  Expression::deepCopy(exp);
  exp->m_defaultValue = Clone(m_defaultValue);
  exp->m_attributeList = Clone(m_attributeList);
  return exp;
}

const std::string ParameterExpression::getOriginalTypeHint() const {
  assert(hasTypeHint());
  return m_originalType->vanillaName();
}

const std::string ParameterExpression::getUserTypeHint() const {
  assert(hasUserType());
  return m_originalType->fullName();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ParameterExpression::parseHandler(ClassScopePtr cls) {
  // Trait has not been 'inlined' into using class so context is not available
  if (!m_type.empty() && !cls->isTrait()) {
    fixupSelfAndParentTypehints(cls);

    if (m_defaultValue) {
      compatibleDefault();
    }
  }
}

void ParameterExpression::fixupSelfAndParentTypehints(ClassScopePtr cls) {
  if (m_type == "self") {
    m_type = cls->getName();
  } else if (m_type == "parent") {
    if (!cls->getOriginalParent().empty()) {
      m_type = Util::toLower(cls->getOriginalParent());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ParameterExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_defaultValue) m_defaultValue->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (!m_type.empty()) {
      addUserClass(ar, m_type);
    }
    // Have to use non const ref params for magic methods
    FunctionScopePtr fs = getFunctionScope();
    if (fs->isMagicMethod() || fs->getName() == "offsetget") {
      fs->getVariables()->addLvalParam(m_name);
    }
  }
}

ConstructPtr ParameterExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_defaultValue;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ParameterExpression::getKidCount() const {
  return 1;
}

void ParameterExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_defaultValue = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      break;
  }
}

TypePtr ParameterExpression::getTypeSpecForClass(AnalysisResultPtr ar,
                                                 bool forInference) {
  TypePtr ret;
  if (forInference) {
    ClassScopePtr cls = ar->findClass(m_type);
    if (!cls || cls->isRedeclaring() || cls->derivedByDynamic()) {
      if (!cls && getScope()->isFirstPass() && !ar->isTypeAliasName(m_type)) {
        ConstructPtr self = shared_from_this();
        Compiler::Error(Compiler::UnknownClass, self);
      }
      ret = Type::Variant;
    }
    if (cls) {
      // Classes must be redeclaring if there are also type aliases
      // with the same name.
      assert(!ar->isTypeAliasName(m_type) || cls->isRedeclaring());
    }
  }
  if (!ret) {
    ret = ar->isTypeAliasName(m_type) || !Option::WholeProgram
      ? Type::Variant
      : Type::CreateObjectType(m_type);
  }
  always_assert(ret);
  return ret;
}

TypePtr ParameterExpression::getTypeSpec(AnalysisResultPtr ar,
                                         bool forInference) {
  const Type::TypePtrMap &types = Type::GetTypeHintTypes(m_hhType);
  Type::TypePtrMap::const_iterator iter;

  TypePtr ret;
  if (m_type.empty()) {
    ret = Type::Some;
  } else if ((iter = types.find(m_type)) != types.end()) {
    ret = iter->second;
  } else {
    ret = getTypeSpecForClass(ar, forInference);
  }

  ConstantExpressionPtr p;
  if (ret->isPrimitive() &&
      m_defaultValue &&
      (p = dynamic_pointer_cast<ConstantExpression>(m_defaultValue)) &&
      p->isNull()) {
    // if we have a primitive type on the LHS w/ a default
    // of null, then don't bother to infer it's type, since we will
    // not specialize for this case
    ret = Type::Some;
  }

  // we still want the above to run, so to record errors and infer defaults
  if (m_ref && forInference) {
    ret = Type::Variant;
  }

  return ret;
}

TypePtr ParameterExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  assert(type->is(Type::KindOfSome) || type->is(Type::KindOfAny));
  TypePtr ret = getTypeSpec(ar, true);

  VariableTablePtr variables = getScope()->getVariables();
  // Functions that can be called dynamically have to have
  // variant parameters, even if they have a type hint
  if ((Option::AllDynamic || getFunctionScope()->isDynamic()) ||
      getFunctionScope()->isRedeclaring() ||
      getFunctionScope()->isVirtual()) {
    if (!Option::HardTypeHints || !ret->isExactType()) {
      variables->forceVariant(ar, m_name, VariableTable::AnyVars);
      ret = Type::Variant;
    }
  }

  if (m_defaultValue && !m_ref) {
    TypePtr r = m_defaultValue->inferAndCheck(ar, Type::Some, false);
    if (!m_defaultValue->is(KindOfConstantExpression) ||
        !static_pointer_cast<ConstantExpression>(m_defaultValue)->isNull()) {
      ret = Type::Coerce(ar, r, ret);
    }
  }

  // parameters are like variables, but we need to remember these are
  // parameters so when variable table is generated, they are not generated
  // as declared variables.
  return variables->addParamLike(m_name, ret, ar, shared_from_this(),
                                 getScope()->isFirstPass());
}

void ParameterExpression::compatibleDefault() {
  bool compat = true;
  if (!m_defaultValue || !hasTypeHint()) return;

  DataType defaultType = KindOfUninit;
  if (m_defaultValue->isArray()) {
    defaultType = KindOfArray;
  } else if (m_defaultValue->isLiteralNull()) {
    defaultType = KindOfNull;
  } else {
    Variant defaultValue;
    if (m_defaultValue->getScalarValue(defaultValue)) {
      defaultType = defaultValue.getType();
    }
  }

  const char* msg = "Default value for parameter %s with type %s "
                    "needs to have the same type as the type hint %s";
  if (m_hhType) {
    // Normally a named type like 'int' is compatable with Int but not integer
    // Since the default value's type is inferred from the value itself it is
    // ok to compare against the lower case version of the type hint in hint
    const char* hint = getTypeHint().c_str();
    switch(defaultType) {
    case KindOfBoolean:
      compat = (!strcmp(hint, "bool") || !strcmp(hint, "boolean")); break;
    case KindOfInt64:
      compat = (!strcmp(hint, "int") || !strcmp(hint, "integer")); break;
    case KindOfDouble:
      compat = (!strcmp(hint, "float") || !strcmp(hint, "double")); break;
    case KindOfString:  /* fall through */
    case KindOfStaticString:
      compat = !strcmp(hint, "string"); break;
    case KindOfArray:
      compat = !strcmp(hint, "array"); break;
    case KindOfUninit:  /* fall through */
    case KindOfNull:    compat = true; break;
    /* KindOfClass is an hhvm internal type, can not occur here */
    case KindOfObject:  /* fall through */
    case KindOfResource: /* fall through */
    case KindOfRef: assert(false /* likely parser bug */);
    default:            compat = false; break;
    }
  } else {
    msg = "Default value for parameter %s with a class type hint "
          "can only be NULL";
    switch(defaultType) {
    case KindOfNull:
      compat = true; break;
    case KindOfArray:
      compat = strcmp(getTypeHint().c_str(), "array") == 0; break;
    default:
      compat = false;
      if (strcmp(getTypeHint().c_str(), "array") == 0) {
        msg = "Default value for parameter %s with array type hint "
              "can only be an array or NULL";
      }
      break;
    }
  }

  if (!compat) {
    string name = getName();
    string tdefault = HPHP::tname(defaultType);
    parseTimeFatal(Compiler::BadDefaultValueType, msg,
                   name.c_str(), tdefault.c_str(),
                   getOriginalTypeHint().c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////

void ParameterExpression::outputCodeModel(CodeGenerator &cg) {
  auto propCount = 2;
  if (m_attributeList) propCount++;
  if (m_modifier != 0) propCount++;
  if (m_ref) propCount++;
  if (m_defaultValue != nullptr) propCount++;
  cg.printObjectHeader("ParameterDeclaration", propCount);
  if (m_attributeList) {
    cg.printPropertyHeader("isPassedByReference");
    cg.printExpressionVector(m_attributeList);
  }
  if (m_modifier != 0) {
    cg.printPropertyHeader("modifiers");
    printf("V:9:\"HH\\Vector\":1:{");
    switch (m_modifier) {
      case T_PUBLIC: cg.printValue("public"); break;
      case T_PROTECTED: cg.printValue("protected"); break;
      case T_PRIVATE: cg.printValue("private"); break;
      default: assert(false);
    }
    printf("}");
  }
  if (m_ref) {
    cg.printPropertyHeader("isPassedByReference");
    cg.printValue(true);
  }
  cg.printPropertyHeader("name");
  cg.printValue(m_name);
  if (m_defaultValue) {
    cg.printPropertyHeader("expression");
    m_defaultValue->outputCodeModel(cg);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ParameterExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (!m_type.empty()) cg_printf("%s ", m_originalType->vanillaName().c_str());
  if (m_ref) cg_printf("&");
  cg_printf("$%s", m_name.c_str());
  if (m_defaultValue) {
    cg_printf(" = ");
    m_defaultValue->outputPHP(cg, ar);
  }
}
