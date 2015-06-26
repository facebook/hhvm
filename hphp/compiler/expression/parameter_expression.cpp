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
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/type_annotation.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/util/text-util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/type-constraint.h"

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
     ExpressionPtr attributeList,
     bool variadic)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ParameterExpression))
  , m_originalType(type)
  , m_name(name)
  , m_hhType(hhType)
  , m_ref(ref)
  , m_modifier(modifier)
  , m_defaultValue(defaultValue)
  , m_attributeList(attributeList)
  , m_variadic(variadic)
{
  m_type = toLower(type ? type->vanillaName() : "");
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

const std::string ParameterExpression::getTypeHintDisplayName() const {
  auto name = m_originalType->vanillaName();
  const char* str = name.c_str();
  auto len = name.size();
  if (len > 3 && tolower(str[0]) == 'h' && tolower(str[1]) == 'h' &&
      str[2] == '\\') {
    bool strip = false;
    const char* stripped = str + 3;
    switch (len - 3) {
      case 3:
        strip = (!strcasecmp(stripped, "int") ||
                 !strcasecmp(stripped, "num"));
        break;
      case 4: strip = !strcasecmp(stripped, "bool"); break;
      case 5: strip = !strcasecmp(stripped, "float"); break;
      case 6: strip = !strcasecmp(stripped, "string"); break;
      case 8: strip = (!strcasecmp(stripped, "resource") ||
                       !strcasecmp(stripped, "arraykey"));
              break;
      default:
        break;
    }
    if (strip) {
      return stripped;
    }
  }
  return name;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ParameterExpression::parseHandler(FileScopeRawPtr file,
                                       ClassScopePtr cls) {
  // Trait has not been 'inlined' into using class so context is not available
  if (!m_type.empty() && !cls->isTrait()) {
    fixupSelfAndParentTypehints(cls);

    if (m_defaultValue) {
      compatibleDefault(file);
    }
  }
}

void ParameterExpression::fixupSelfAndParentTypehints(ClassScopePtr cls) {
  if (m_type == "self") {
    m_type = toLower(cls->getOriginalName());
  } else if (m_type == "parent") {
    if (!cls->getOriginalParent().empty()) {
      m_type = toLower(cls->getOriginalParent());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ParameterExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_defaultValue) m_defaultValue->analyzeProgram(ar);
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

static bool useHackTypeHintErrorMessage(const char* hint) {
  String typeName(hint);
  MaybeDataType dt = nameToMaybeDataType(typeName.get());
  if (!dt.hasValue()) {
    return false;
  }
  switch (*dt) {
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfString:
    case KindOfResource:
      return true;
    default:
      return false;
  }
}

void ParameterExpression::compatibleDefault(FileScopeRawPtr file) {
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

  // Normally a named type like 'int' is compatible with Int but not integer
  // Since the default value's type is inferred from the value itself it is
  // ok to compare against the lower case version of the type hint in hint
  const char* hint = getTypeHint().c_str();
  [&] {
    switch (defaultType) {
      case KindOfUninit:
        compat = m_hhType;
        return;
      case KindOfNull:
        compat = true;
        return;

      case KindOfBoolean:
        compat = !strcasecmp(hint, "HH\\bool");
        return;

      case KindOfInt64:
        compat = (!strcasecmp(hint, "HH\\int") ||
                  !strcasecmp(hint, "HH\\num") ||
                  !strcasecmp(hint, "HH\\arraykey") ||
                  (m_hhType && interface_supports_int(hint)));
        return;

      case KindOfDouble:
        compat = (!strcasecmp(hint, "HH\\float") ||
                  !strcasecmp(hint, "HH\\num") ||
                  (m_hhType && interface_supports_double(hint)));
        return;

      case KindOfStaticString:
      case KindOfString:
        compat = (!strcasecmp(hint, "HH\\string") ||
                  !strcasecmp(hint, "HH\\arraykey") ||
                  (m_hhType && interface_supports_string(hint)));
        return;

      case KindOfArray:
        compat = (!strcasecmp(hint, "array") ||
                  (m_hhType && interface_supports_array(hint)));
        return;

      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
        if (!m_hhType) {
          compat = false;
          return;
        }
      /* fall through */
      case KindOfClass:
        break;
    }
    always_assert(false /* likely parser bug */);
  }();

  if (!compat) {
    const char* msg = "Default value for parameter %s with type %s "
                      "needs to have the same type as the type hint %s";
    if (!m_hhType) {
      if (!strcasecmp(hint, "array")) {
        msg = "Default value for parameter %s with array type hint "
              "can only be an array or NULL";
      } else if (!useHackTypeHintErrorMessage(hint)) {
        msg = "Default value for parameter %s with a class type hint "
              "can only be NULL";
       }
    }

    string name = getName();
    string tdefault = HPHP::tname(defaultType);
    parseTimeFatal(file,
                   Compiler::BadDefaultValueType, msg,
                   name.c_str(), tdefault.c_str(),
                   getTypeHintDisplayName().c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////

void ParameterExpression::outputCodeModel(CodeGenerator &cg) {
  auto propCount = 2;
  if (m_attributeList) propCount++;
  if (m_modifier != 0) propCount++;
  if (m_originalType) propCount++;
  if (m_ref) propCount++;
  if (m_defaultValue != nullptr) propCount++;
  cg.printObjectHeader("ParameterDeclaration", propCount);
  if (m_attributeList) {
    cg.printPropertyHeader("attributes");
    cg.printExpressionVector(m_attributeList);
  }
  if (m_modifier != 0) {
    cg.printPropertyHeader("modifiers");
    cg.printf("V:9:\"HH\\Vector\":1:{");
    cg.printObjectHeader("Modifier", 1);
    cg.printPropertyHeader("name");
    switch (m_modifier) {
      case T_PUBLIC: cg.printValue("public"); break;
      case T_PROTECTED: cg.printValue("protected"); break;
      case T_PRIVATE: cg.printValue("private"); break;
      default: assert(false);
    }
    cg.printObjectFooter();
    cg.printf("}");
  }
  if (m_originalType) {
    cg.printPropertyHeader("typeAnnotation");
    m_originalType->outputCodeModel(cg);
  }
  if (m_ref) {
    cg.printPropertyHeader("isPassedByReference");
    cg.printBool(true);
  }
  cg.printPropertyHeader("name");
  cg.printValue(m_name);
  if (m_defaultValue) {
    cg.printPropertyHeader("expression");
    m_defaultValue->outputCodeModel(cg);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
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
