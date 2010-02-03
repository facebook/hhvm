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

#include <lib/expression/object_property_expression.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/variable_table.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/option.h>
#include <lib/expression/simple_variable.h>
#include <util/hash.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectPropertyExpression::ObjectPropertyExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr property)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_object(object), m_property(property), m_valid(false), m_static(false) {
  m_object->setContext(Expression::ObjectContext);
}

ExpressionPtr ObjectPropertyExpression::clone() {
  ObjectPropertyExpressionPtr exp(new ObjectPropertyExpression(*this));
  Expression::deepCopy(exp);
  exp->m_object = Clone(m_object);
  exp->m_property = Clone(m_property);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ObjectPropertyExpression::setContext(Context context) {
  m_context |= context;
  if (context == Expression::LValue &&
      !hasContext(Expression::UnsetContext)) {
    m_object->setContext(Expression::LValue);
  }
}
void ObjectPropertyExpression::clearContext(Context context) {
  m_context &= ~context;
  if (context == Expression::LValue) {
    m_object->clearContext(Expression::LValue);
  }
}

void ObjectPropertyExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_object->analyzeProgram(ar);
  m_property->analyzeProgram(ar);
}

ExpressionPtr ObjectPropertyExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_object);
  ar->preOptimize(m_property);
  return ExpressionPtr();
}

ExpressionPtr ObjectPropertyExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_object);
  ar->postOptimize(m_property);
  return ExpressionPtr();
}

TypePtr ObjectPropertyExpression::inferTypes(AnalysisResultPtr ar,
                                             TypePtr type, bool coerce) {
  m_valid = false;

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, NEW_TYPE(Object), true);

  if (!m_property->is(Expression::KindOfScalarExpression)) {
    // if dynamic property or method, we have nothing to find out
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UseDynamicProperty, self);
    }
    m_property->inferAndCheck(ar, Type::String, false);

    // we also lost track of which class variable an expression is about, hence
    // any type inference could be wrong. Instead, we just force variants on
    // all class variables.
    if (m_context & (LValue | RefValue)) {
      ar->forceClassVariants();
    }

    return Type::Variant; // we have to use a variant to hold dynamic value
  }

  ScalarExpressionPtr exp = dynamic_pointer_cast<ScalarExpression>(m_property);
  string name = exp->getString();
  ASSERT(!name.empty());

  ClassScopePtr cls;
  if (objectType && !objectType->getName().empty()) {
    // what object-> has told us
    cls = ar->findClass(objectType->getName());
    ASSERT(cls);
  } else {
    // what ->property has told us
    cls = ar->findClass(name, AnalysisResult::PropertyName);
    if (!cls) {
      if (m_context & (LValue | RefValue)) {
        ar->forceClassVariants(name);
      }
      return Type::Variant;
    }

    m_object->inferAndCheck(ar, Type::CreateObjectType(cls->getName()), true);
  }

  // resolved to this class
  int present = 0;
  if (m_context & RefValue) {
    type = Type::Variant;
    coerce = true;
  }

  // use $this inside a static function
  if (m_object->isThis()) {
    FunctionScopePtr func = ar->getFunctionScope();
    if (func->isStatic()) {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(self, CodeError::MissingObjectContext,
                                   self);
      }
      m_actualType = Type::Variant;
      return m_actualType;
    }
  }

  TypePtr ret;
  if (!cls->derivesFromRedeclaring()) { // Have to use dynamic.
    ret = cls->checkProperty(name, type, coerce, ar, self, present);
    if (present) {
      m_valid = true;
      m_static = present & VariableTable::VariableStatic;
      if (m_static) {
        ar->getScope()->getVariables()->
          setAttribute(VariableTable::NeedGlobalPointer);
      }
      m_class = cls;
    }
  }

  // get() will return Variant
  if (!m_valid || !m_object->getType()->isSpecificObject()) {
    m_actualType = Type::Variant;
    return m_actualType;
  }
  if (ar->getPhase() == AnalysisResult::LastInference) {
    if (!(m_context & ObjectContext)) {
      m_object->clearContext(Expression::LValue);
    }
    setContext(Expression::NoLValueWrapper);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ObjectPropertyExpression::outputPHP(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  m_object->outputPHP(cg, ar);
  cg.printf("->");
  if (m_property->getKindOf() == Expression::KindOfScalarExpression) {
    m_property->outputPHP(cg, ar);
  } else {
    cg.printf("{");
    m_property->outputPHP(cg, ar);
    cg.printf("}");
  }
}

bool ObjectPropertyExpression::directVariantProxy(AnalysisResultPtr ar) {
  TypePtr actualType = m_object->getActualType();
  if (actualType && actualType->is(Type::KindOfVariant)) {
    if (m_object->is(KindOfSimpleVariable)) {
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(m_object);
      const std::string &name = var->getName();
      FunctionScopePtr func =
        dynamic_pointer_cast<FunctionScope>(ar->getScope());
      VariableTablePtr variables = func->getVariables();
      if (!variables->isParameter(name) || variables->isLvalParam(name)) {
        return true;
      }
      if (variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
          variables->getAttribute(VariableTable::ContainsExtract)) {
        return true;
      }
    } else {
      return true;
    }
  }
  return false;
}

void ObjectPropertyExpression::outputCPPImpl(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  bool bThis = m_object->isThis();

  const char *op = ".";
  string func = Option::ObjectPrefix;
  if (bThis && ar->getFunctionScope()->isStatic()) {
    func = Option::ObjectStaticPrefix;
  }
  if (m_context & (LValue | RefValue)) {
    func += "lval";
  } else {
    func += "get";
  }

  if (m_property->getKindOf() == Expression::KindOfScalarExpression) {
    ScalarExpressionPtr name =
      dynamic_pointer_cast<ScalarExpression>(m_property);
    const char *propName = name->getString().c_str();
    if (m_valid && m_object->getType()->isSpecificObject()) {
      if (m_static) {
        if (!bThis) {
          ASSERT(m_class);
          cg.printf("g->%s%s%s%s",
                    Option::StaticPropertyPrefix, m_class->getName().c_str(),
                    Option::IdPrefix.c_str(), propName);
        } else {
          // if $val is a class static variable (static $val), then $val
          // cannot be declared as a class variable (var $val), $this->val
          // refers to a non-static class variable and has to use get/lval.
          uint64 hash = hash_string(propName);
          cg.printf("%s(\"%s\", 0x%016llXLL)", func.c_str(), propName, hash);
        }
      } else {
        if (!bThis) {
          if (directVariantProxy(ar)) {
            TypePtr expectedType = m_object->getExpectedType();
            ASSERT(expectedType->is(Type::KindOfObject));
            // Clear m_expectedType to avoid type cast (toObject).
            m_object->setExpectedType(TypePtr());
            m_object->outputCPP(cg, ar);
            m_object->setExpectedType(expectedType);
            cg.printf(".");
          } else {
            m_object->outputCPP(cg, ar);
            cg.printf("->");
          }
        }
        cg.printf("%s%s", Option::PropertyPrefix, propName);
      }
    } else {
      if (!bThis) {
        if (directVariantProxy(ar)) {
          TypePtr expectedType = m_object->getExpectedType();
          ASSERT(expectedType->is(Type::KindOfObject));
          // Clear m_expectedType to avoid type cast (toObject).
          m_object->setExpectedType(TypePtr());
          m_object->outputCPP(cg, ar);
          m_object->setExpectedType(expectedType);
        } else {
          m_object->outputCPP(cg, ar);
        }
        cg.printf(op);
      }
      uint64 hash = hash_string(propName);
      cg.printf("%s(\"%s\", 0x%016llXLL)", func.c_str(), propName, hash);
    }
  } else {
    if (!bThis) {
      if (directVariantProxy(ar)) {
        TypePtr expectedType = m_object->getExpectedType();
        ASSERT(expectedType->is(Type::KindOfObject));
        // Clear m_expectedType to avoid type cast (toObject).
        m_object->setExpectedType(TypePtr());
        m_object->outputCPP(cg, ar);
        m_object->setExpectedType(expectedType);
      } else {
        m_object->outputCPP(cg, ar);
      }
      cg.printf(op);
    }
    cg.printf("%s(", func.c_str());
    m_property->outputCPP(cg, ar);
    cg.printf(", -1LL)");
  }
}

void ObjectPropertyExpression::outputCPPIsset(CodeGenerator &cg,
                                              AnalysisResultPtr ar) {
  bool bThis = m_object->isThis();
  if (!bThis) {
    m_object->outputCPP(cg, ar);
    cg.printf("->");
  }
  cg.printf("t___isset(");
  bool direct = m_property->getKindOf() == Expression::KindOfScalarExpression;
  if (direct) {
    cg.printf("\"");
  }
  m_property->outputCPP(cg, ar);
  if (direct) {
    cg.printf("\"");
  }
  cg.printf(")");
}
void ObjectPropertyExpression::outputCPPUnset(CodeGenerator &cg,
                                              AnalysisResultPtr ar) {
  bool bThis = m_object->isThis();
  if (!bThis) {
    m_object->outputCPP(cg, ar);
    cg.printf("->");
  }
  cg.printf("t___unset(");
  bool direct = m_property->getKindOf() == Expression::KindOfScalarExpression;
  if (direct) {
    cg.printf("\"");
  }
  m_property->outputCPP(cg, ar);
  if (direct) {
    cg.printf("\"");
  }
  cg.printf(");\n");
}
