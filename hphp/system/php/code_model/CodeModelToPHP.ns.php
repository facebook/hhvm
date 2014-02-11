<?php
// Copyright 2004-present Facebook. All Rights Reserved.
namespace HH\CodeModel {

/** A visitor that converts Code Model instances in PHP source strings. */
class CodeModelToPHP {
  private /*string*/ $indent = '';
  private /*string*/ $blockIndent = '';

  private function visitVector(
    /*Vector<string>*/ $elements,
    /*string*/ $left_delimiter,
    /*string*/ $separator,
    /*string*/ $right_delimiter,
    /*string*/ $empty_value = ''
  ) /*: string*/ {
    if ($elements->count() == 0) { return $empty_value; }
    $result = $left_delimiter;
    foreach ($elements as $elem) {
      if ($result !== $left_delimiter) { $result .= $separator; }
      $result .= $elem;
    }
    return $result.$right_delimiter;
  }

  /**
   *  <?(php|hh)
   *
   *  statements
   */
  public function visitScript(
    /*IBlockStatement*/ $node,
    /*string*/ $hh = 'hh'
  ) /*: mixed*/ {
    $result = "<?$hh\n\n";
    $statements = Vector {};
    foreach ($node->getStatements() as $statements_elem) {
      $statements[] = /*(string)*/$statements_elem->accept($this);
    }

    $result .= $this->visitVector($statements, '', '', '');
    return $result;
  }

  /**
   *  attributeName [( expressions )]
   */
  public function visitAttribute(
    /*IAttribute*/ $node) /*: mixed*/ {
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $result = $node->getAttributeName();
    $result .= $this->visitVector($expressions, '(', ', ', ')', '');
    return $result;
  }

  /**
   *  A namespace function or a class method.
   */
  public function visitFunctionData(
    /*IFunctionData*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $type_arguments = Vector {};
    foreach ($node->getTypeArguments() as $type_arguments_elem) {
      $type_arguments[] =
        /*(string)*/$type_arguments_elem->acceptAsReference($this);
    }
    $parameters = Vector {};
    foreach ($node->getParameters() as $parameters_elem) {
      $parameters[] = /*(string)*/$parameters_elem->accept($this);
    }
    $captured_variables = Vector {};
    foreach ($node->getCapturedVariables() as $captured_variables_elem) {
      $captured_variables[] =
        /*(string)*/$captured_variables_elem->accept($this);
    }
    $return_type = null;
    $return_type_value = $node->getReturnType();
    if ($return_type_value !== null) {
      $return_type = /*(string)*/$return_type_value->acceptAsReference($this);
    }
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    $result = $this->visitVector($node->getComments(), $this->indent,
      "\n".$this->indent, "\n".$this->indent, '');
    if ($captured_variables->count() > 0) {
      $result = "{\n";
      $this->indent .= '  ';
      $result .= $this->visitVector($captured_variables,
        $this->indent, ";\n".$this->indent, ";\n");
      $result .= "\n";
    }
    $result .= $this->visitVector($attributes,
      '<< ', ' ', " >>\n".$this->indent, '');
    $result .= $this->visitVector($modifiers, '', ' ', ' ', '');
    $result .= 'function '.$node->getName();
    $result .= $this->visitVector($type_arguments, '<', ', ', '>');
    $result .= $this->visitVector($parameters, '(', ', ', ')', '()');
    if ($return_type !== null) { $result .= ' : './*(string)*/$return_type; }
    $result .= $block;
    if ($captured_variables->count() > 0) {
      $this->indent = /*(string)*/substr($this->indent, 2);
      $result .= "\n".$this->indent.'}';
    }
    $result .= "\n";
    return $result;
  }

  private function visitValue(mixed $value) /*: string*/ {
    return fb_json_encode($value);
  }

  /**
   * A variable whose scope is confined to a function body and
   * whose life time is one call/activation of the function.
   */
  public function visitLocalVariableData(
    /*ILocalVariableData*/ $node) /*: mixed*/ {
    $result = $this->visitVector($node->getComments(), $this->indent,
      "\n".$this->indent, "\n".$this->indent, $this->indent);
    $type = $node->getType();
    if ($type !== null) {
      $result .= /*(string)*/$type->acceptAsReference($this);
      $result .= ' ';
    }
    $result .= '$'.$node->getName();
    $value = $node->getValue();
    if ($value !== null) {
      $result .= ' = '.$this->visitValue($value);
    }
    return $result;
  }

  /**
   * A name that identifies some kind of modification to the
   * semantics of another code node. For example "public".
   */
  public function visitModifier(
    /*IModifier*/ $node) /*: mixed*/ {
    return $node->getName();
  }

  /**
   * A namespace is a named container of nested namespaces, namespace types and
   * namespace functions
   */
  public function visitNamespaceData(
    /*INamespaceData*/ $node) /*: mixed*/ {
    $nested_namespaces = Vector {};
    foreach ($node->getNestedNamespaces() as $nested_namespaces_elem) {
      $nested_namespaces[] = /*(string)*/$nested_namespaces_elem->accept($this);
    }
    $types = Vector {};
    foreach ($node->getTypes() as $types_elem) {
      $types[] = /*(string)*/$types_elem->accept($this);
    }
    $functions = Vector {};
    foreach ($node->getFunctions() as $functions_elem) {
      $functions[] = /*(string)*/$functions_elem->accept($this);
    }

    $result = $this->visitVector($node->getComments(), $this->indent,
      "\n".$this->indent, "\n".$this->indent, '');
    $result .= 'namespace '.$node->getName()." {\n";
    $this->indent .= '  ';
    $result .= $this->visitVector(
      $functions, $this->indent, "\n\n".$this->indent, "\n\n");
    $result .= $this->visitVector(
      $types, $this->indent, "\n\n".$this->indent, "\n\n");
    $result .= $this->visitVector(
      $nested_namespaces, $this->indent, "\n\n".$this->indent, "\n\n");
    $this->indent = /*(string)*/substr($this->indent, 2);
    $result .= "\n".$this->indent.'}';

    return $result;
  }

  /**
   * A parameter is a name, possibly annoted with a type, for a
   * runtime argument value supplied by the caller of a parameterized function.
   */
  public function visitParameterData(
    /*IParameterData*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $type = null;
    $type_value = $node->getType();
    if ($type_value !== null) {
      $type = /*(string)*/$type_value->acceptAsReference($this);
    }

    $result = $this->visitVector($attributes, '<< ', ' ', ' >> ', '');
    if (is_string($type)) {
      $result .= $type.' ';
    }
    $result .= $node->getName();
    $value = $node->getValue();
    if ($value !== null) {
      $result .= ' = '.$this->visitValue($value);
    }
    return $result;
  }

  /**
   * A property is a name, possibly annoted with a type, for
   * a runtime value that forms part of an object or array.
   */
  public function visitPropertyData(
    /*IPropertyData*/ $node) /*: mixed*/ {

    $result = $this->visitVector($node->getComments(), $this->indent,
      "\n".$this->indent, "\n".$this->indent, $this->indent);
    $type = $node->getType();
    if ($type !== null) {
      $result .= /*(string)*/$type->acceptAsReference($this);
      $result .= ' ';
    }
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $result .= '$'.$node->getName();
    $value = $node->getValue();
    if ($value !== null) {
      $result .= ' = '.$this->visitValue($value);
    }
    return $result;
  }

  /**
   * A type is the metadata for a set of values and a set of operations that
   * may be applied to the values in the set.
   */
  public function visitTypeData(
    /*ITypeData*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $type_arguments = Vector {};
    foreach ($node->getTypeArguments() as $type_arguments_elem) {
      $type_arguments[] =
        /*(string)*/$type_arguments_elem->acceptAsReference($this);
    }
    $base_types = Vector {};
    foreach ($node->getBaseTypes() as $base_types_elem) {
      $base_types[] = /*(string)*/$base_types_elem->acceptAsReference($this);
    }
    $interfaces = Vector {};
    foreach ($node->getInterfaces() as $interfaces_elem) {
      $interfaces[] = /*(string)*/$interfaces_elem->acceptAsReference($this);
    }
    $methods = Vector {};
    foreach ($node->getMethods() as $methods_elem) {
      $methods[] = /*(string)*/$methods_elem->accept($this);
    }
    $properties = Vector {};
    foreach ($node->getProperties() as $properties_elem) {
      $properties[] = /*(string)*/$properties_elem->accept($this);
    }

    $result = $this->visitVector($node->getComments(), $this->indent,
      "\n".$this->indent, "\n".$this->indent, '');
    $result .= $this->visitVector($attributes,
      '<< ', ' ', " >>\n".$this->indent, '');
    $result .= $this->visitVector($modifiers, '', ' ', ' ');
    if ($node->getIsClass()) {
      $result .= 'class ';
    } else if ($node->getIsInterface()) {
      $result .= 'interface ';
    } else {
      $result .= 'trait ';
    }
    $result .= $node->getName();
    $result .= $this->visitVector($type_arguments, '<', ', ', '> ', ' ');
    $result .= $this->visitVector($base_types, 'extends ', ', ', ' ');
    $result .= $this->visitVector(
      $interfaces, "\n".$this->indent.'implements ', ', ', ' ');
    $result .= "{\n";
    $this->indent .= '  ';
    $result .= $this->visitVector(
      $properties, $this->indent, "\n".$this->indent, "\n\n");
    $result .= $this->visitVector(
      $methods, $this->indent, "\n\n".$this->indent, "\n\n");
    $this->indent = /*(string)*/substr($this->indent, 2);
    $result .= "\n".$this->indent.'}';

    return $result;
  }

  /**
   *  Given (x) op y return true if x contains an operator of lower priority
   *  than op, with the implication that removing the parentheses around x
   *  will change the evaluation order of the overall expression.
   */
  private function leftExprNeedsParentheses(
    /*BinaryOperator*/ $op, /*IExpression*/ $expr) /* : bool */ {
    if ($expr instanceof ConditionalExpression) {
      // let (x) == (a ? b : c)
      // (a ? b : c) op y != a ? b : c op y in all cases
      return true;
    }
    if ($expr instanceof BinaryOpExpression) {
      // let (x) == (a op1 b)
      // (a op1 b) op y != a op1 b op y if
      if (BinaryOperators::hasPriority($op, $expr->getOperation())) {
        return true;
      }
      // check if a op1 (b) op y == a op1 b op y
      return $this->leftExprNeedsParentheses($op, $expr->getExpression2());
    }
    if ($expr instanceof UnaryOpExpression) {
      // let (x) == (!a)
      // (!a) op y != !a op y if
      if ($expr->getOperation() == UnaryOperators::PHP_NOT_OP &&
          BinaryOperators::hasPriority(
            $op, BinaryOperators::PHP_MULTIPLY)) {
        return true;
      }
      // check if !(a) op y == !a op y
      return $this->leftExprNeedsParentheses($op, $expr->getExpression());
    }
    return false;
  }

  /**
   *  Given x op (y) return true if y contains an operator of lower priority
   *  than op, with the implication that removing the parentheses around y
   *  will change the evaluation order of the overall expression.
   */
  private function rightExprNeedsParentheses(
    /*BinaryOperator*/ $op, /*IExpression*/ $expr) /* : bool */ {
    if ($op == BinaryOperators::PHP_ARRAY_ELEMENT) {
      // x[y] == x[(y)]
      return false;
    }
    if ($expr instanceof ConditionalExpression) {
      // let (y) == (a ? b : c)
      // x op (a ? b : c) != x op a ? b : c if
      return $this->rightExprNeedsParentheses(
        $op, $expr->getCondition());
    }
    if ($expr instanceof BinaryOpExpression) {
      // let (y) == (a op1 b)
      // x op (a op1 b) != x op a op1 b if
      if (!BinaryOperators::hasPriority($expr->getOperation(), $op)) {
        return true;
      }
      // check if x op (a) op1 b == x op a op1 b
      return $this->rightExprNeedsParentheses($op, $expr->getExpression1());
    }

    // don't worry about unary operations:
    // prefix unaries do not have left operands hence
    // x op (uop a) == x op uop a in all cases.
    // postfix unaries trump binaries that get here hence
    // x op (a uop) == x op a uop in all cases.
    return false;
  }

  /**
   *  expression1 operation expression2
   */
  public function visitBinaryOpExpression(
    /*IBinaryOpExpression*/ $node) /*: mixed*/ {
    $expression1 = /*(string)*/$node->getExpression1()->accept($this);
    $expression2 = /*(string)*/$node->getExpression2()->accept($this);
    $op = $node->getOperation();
    $expr1 = $node->getExpression1();
    $expr2 = $node->getExpression2();

    if ($this->leftExprNeedsParentheses($op, $expr1)) {
      $expression1 = '('.$expression1.')';
    }
    $r = $this->visitVector($node->getComments(), '', '', '');
    $r .= $expression1;
    switch ($op) {
      case BinaryOperators::PHP_AND_ASSIGN: $r .= ' &= '; break;
      case BinaryOperators::PHP_AND: $r .= ' & '; break;
      case BinaryOperators::PHP_ARRAY_ELEMENT: $r .= '['; break;
      case BinaryOperators::PHP_ARRAY_PAIR: $r .= ' => '; break;
      case BinaryOperators::PHP_ASSIGNMENT: $r .= ' = '; break;
      case BinaryOperators::PHP_BOOLEAN_AND: $r .= ' && '; break;
      case BinaryOperators::PHP_BOOLEAN_OR: $r .= ' || '; break;
      case BinaryOperators::PHP_CONCAT_ASSIGN: $r .= ' .= '; break;
      case BinaryOperators::PHP_CONCAT: $r .= '.'; break;
      case BinaryOperators::PHP_CONDITIONAL: $r .= ' ?: '; break;
      case BinaryOperators::PHP_DIVIDE_ASSIGN: $r .= ' /= '; break;
      case BinaryOperators::PHP_DIVIDE: $r .= ' / '; break;
      case BinaryOperators::PHP_INSTANCEOF: $r .= ' instanceof '; break;
      case BinaryOperators::PHP_IS_EQUAL: $r .= ' == '; break;
      case BinaryOperators::PHP_IS_GREATER: $r .= ' > '; break;
      case BinaryOperators::PHP_IS_GREATER_OR_EQUAL: $r .= ' >= '; break;
      case BinaryOperators::PHP_IS_IDENTICAL: $r .= ' === '; break;
      case BinaryOperators::PHP_IS_NOT_IDENTICAL: $r .= ' !== '; break;
      case BinaryOperators::PHP_IS_NOT_EQUAL: $r .= ' != '; break;
      case BinaryOperators::PHP_IS_SMALLER: $r .= ' < '; break;
      case BinaryOperators::PHP_IS_SMALLER_OR_EQUAL: $r .= ' <= '; break;
      case BinaryOperators::PHP_LOGICAL_AND: $r .= ' and '; break;
      case BinaryOperators::PHP_LOGICAL_OR: $r .= ' or '; break;
      case BinaryOperators::PHP_LOGICAL_XOR: $r .= ' xor '; break;
      case BinaryOperators::PHP_MINUS_ASSIGN: $r .= ' -= '; break;
      case BinaryOperators::PHP_MINUS: $r .= ' - '; break;
      case BinaryOperators::PHP_MODULUS_ASSIGN: $r .= ' %= '; break;
      case BinaryOperators::PHP_MODULUS: $r .= ' % '; break;
      case BinaryOperators::PHP_MULTIPLY_ASSIGN: $r .= ' *= '; break;
      case BinaryOperators::PHP_MULTIPLY: $r .= ' * '; break;
      case BinaryOperators::PHP_OR_ASSIGN: $r .= ' |= '; break;
      case BinaryOperators::PHP_OR: $r .= ' | '; break;
      case BinaryOperators::PHP_PLUS_ASSIGN: $r .= ' += '; break;
      case BinaryOperators::PHP_PLUS: $r .= ' + '; break;
      case BinaryOperators::PHP_SHIFT_LEFT_ASSIGN: $r .= ' <<= '; break;
      case BinaryOperators::PHP_SHIFT_LEFT: $r .= ' << '; break;
      case BinaryOperators::PHP_SHIFT_RIGHT_ASSIGN: $r .= ' >>= '; break;
      case BinaryOperators::PHP_SHIFT_RIGHT: $r .= ' >> '; break;
      case BinaryOperators::PHP_XOR_ASSIGN: $r .= ' ^= '; break;
      case BinaryOperators::PHP_XOR: $r .= ' ^ '; break;
    }
    if ($op == BinaryOperators::PHP_ARRAY_ELEMENT) {
      $expression2 .= ']';
    }
    if ($this->rightExprNeedsParentheses($op, $expr2)) {
      $expression2 = '('.$expression2.')';
    }
    $r .= $expression2;
    return $r;
  }

  /**
   *  (class|self|parent|static)::constantName
   */
  public function visitClassConstantExpression(
    /*IClassConstantExpression*/ $node) /*: mixed*/ {

    $class = /*(string)*/$node->getClass()->accept($this);
    $result = $class.'::'.$node->getConstantName();
    return $result;
  }

  /**
   *  (class|self|parent|static)::methodName(arguments)
   *  (class|self|parent|static)::{methodExpression}(arguments)
   *  ${classExpression}::methodName(arguments)
   *  ${classExpression}::{methodExpression}(arguments)
   */
  public function visitClassMethodCallExpression(
    /*IClassMethodCallExpression*/ $node) /*: mixed*/ {
    $class = null;
    $class_value = $node->getClass();
    if ($class_value !== null) {
      $class = /*(string)*/$class_value->accept($this);
    }
    $class_expression = null;
    $class_expression_value = $node->getClassExpression();
    if ($class_expression_value !== null) {
      $class_expression = /*(string)*/$class_expression_value->accept($this);
    }
    $method_expression = null;
    $method_expression_value = $node->getMethodExpression();
    if ($method_expression_value !== null) {
      $method_expression = /*(string)*/$method_expression_value->accept($this);
    }
    $arguments = Vector {};
    foreach ($node->getArguments() as $arguments_elem) {
      $arguments[] = /*(string)*/$arguments_elem->accept($this);
    }

    $result = '';
    if (is_string($class)) {
      $result = $class.'::';
    } else if (is_string($class_expression)) {
      $result = $class_expression.'::';
    }
    $method_name = $node->getMethodName();
    if ($method_name !== null) {
      $result .= $method_name;
    } else if (is_string($method_expression)) {
      $result .= '{'.$method_expression.'}';
    }

    $result .= $this->visitVector($arguments, '(', ', ', ')', '()');
    return $result;
  }

  /**
   *  (class|self|parent|static)::$propertyName
   *  (class|self|parent|static)::${expression}
   */
  public function visitClassPropertyExpression(
    /*IClassPropertyExpression*/ $node) /*: mixed*/ {
    $class = null;
    $class_value = $node->getClass();
    if ($class_value !== null) {
      $class = /*(string)*/$class_value->accept($this);
    }
    $class_expression = null;
    $class_expression_value = $node->getClassExpression();
    if ($class_expression_value !== null) {
      $class_expression = /*(string)*/$class_expression_value->accept($this);
    }
    $property_expression = null;
    $property_expression_value = $node->getPropertyExpression();
    if ($property_expression_value !== null) {
      $property_expression =
        /*(string)*/$property_expression_value->accept($this);
    }

    $result = '';
    if (is_string($class)) {
      $result = $class.'::$';
    } else if (is_string($class_expression)) {
      $result = $class_expression.'::$';
    }
    $property_name = $node->getPropertyName();
    if ($property_name !== null) {
      $result .= $property_name;
    } else if (is_string($property_expression)) {
      $result .= '{'.$property_expression.'}';
    }

    return $result;
  }

  /**
   *  function (params) use (vars) { statements }
   */
  public function visitClosureExpression(
    /*IClosureExpression*/ $node) /*: mixed*/ {
    $function = /*(string)*/$node->getFunction()->accept($this);
    $captured_variables = Vector {};
    foreach ($node->getCapturedVariables() as $captured_variables_elem) {
      $captured_variables[] =
        /*(string)*/$captured_variables_elem->accept($this);
    }

    $result = $function;
    $captures =
      $this->visitVector($captured_variables, ' use (', ',', ') {', '');
    if ($captures !== '') {
      $start = strpos($result, ' {');
      $result = substr_replace($result, $captures, $start, 2);
    }
    return $result;
  }

  /**
   *  class {arguments}
   */
  public function visitCollectionInitializerExpression(
    /*ICollectionInitializerExpression*/ $node) /*: mixed*/ {
    $class = /*(string)*/$node->getClass()->accept($this);
    $arguments = Vector {};
    foreach ($node->getArguments() as $arguments_elem) {
      $arguments[] = /*(string)*/$arguments_elem->accept($this);
    }

    $result = $class;
    $result .= $this->visitVector($arguments, '{', ', ', '}', '{}');
    return $result;
  }

  /**
   *   condition ? valueIfTrue : valueIfFalse
   */
  public function visitConditionalExpression(
    /*IConditionalExpression*/ $node) /*: mixed*/ {
    $cond = $node->getCondition();
    $condition = /*(string)*/$cond->accept($this);
    $value_if_true = /*(string)*/$node->getValueIfTrue()->accept($this);
    $val_if_false = $node->getValueIfFalse();
    $value_if_false = /*(string)*/$val_if_false->accept($this);

    if ($this->leftExprNeedsParentheses(
      BinaryOperators::PHP_CONDITIONAL, $cond)) {
      $condition = '('.$condition.')';
    }
    if ($val_if_false instanceof ConditionalExpression) {
      $value_if_false = '('.$value_if_false.')';
    }
    return $condition.' ? '.$value_if_true.' : '.$value_if_false;
  }

  private function isSimpleEncapsExpr(/*IExpression*/ $expr) /* : bool */ {
    if ($expr instanceof SimpleVariableExpression) {
      return true;
    }
    if ($expr instanceof ObjectPropertyExpression) {
      $obj = $expr->getObject();
      return $obj instanceof ISimpleVariableExpression;
    }
    if ($expr instanceof BinaryOpExpression) {
      $arr = $expr->getExpression1();
      if (!($arr instanceof SimpleVariableExpression)) {
        return false;
      }
      $op = $expr->getOperation();
      if ($op !== BinaryOperators::PHP_ARRAY_ELEMENT) {
        return false;
      }
      $index = $expr->getExpression2();
      if ($index instanceof SimpleVariableExpression) {
        return true;
      }
      if ($index instanceof ScalarExpression) {
        $val = $index->getValue();
        return is_string($val) || is_int($val) && $val >= 0;
      }
      return false;
    }
    if ($expr instanceof UnaryOpExpression) {
      $op = $expr->getOperation();
      return $op != UnaryOperators::PHP_DYNAMIC_VARIABLE_OP;
    }
    return false;
  }

  private function escapeForDoubleQuoted(/*string*/ $str) /*: string*/ {
    $len = strlen($str);
    $result = '';
    for ($i = 0; $i < $len; $i++) {
      $c = $str[$i];
      switch ($c) {
        case "\n": $result .= '\\n'; break;
        case "\r": $result .= '\\r'; break;
        case "\t": $result .= '\\t'; break;
        case "\v": $result .= '\\v'; break;
        case "\e": $result .= '\\e'; break;
        case "\f": $result .= '\\f'; break;
        case '\\': $result .= '\\\\'; break;
        case '$':  $result .= '\\$'; break;
        case '"':  $result .= '\\"'; break;
        case '\'': $result .= "'"; break;
        default: {
          if ($c >= ' ' && $c <= '~') {
            // The range [' ', '~'] contains only printable characters
            $result .= /*(string)*/$c;
          } else {
            $dquote = true;
            $oct = /*(string)*/sprintf('%o', ord($c));
            $result .= '\\'.$oct;
          }
          break;
        }
      }
    }
    return $result;
  }

  private function escapeForSingleQuoted(/*string*/ $str) /*: string*/ {
    $len = strlen($str);
    $result = '';
    for ($i = 0; $i < $len; $i++) {
      $c = $str[$i];
      switch ($c) {
        case '\\': $result .= '\\\\'; break;
        case '\'': $result .= "'"; break;
        default:   $result .= /*(string)*/$c; break;
      }
    }
    return $result;
  }

  /**
   * `text $var more text ...`
   * "text $var more text ..."
   * 'text ...'
   * <<<Delimiter text $var more text ... Delimiter;
   * <<<'Delimiter' text ... Delimiter;
   */
  public function visitEncapsListExpression(
    /*IEncapsListExpression*/ $node) /*: mixed*/ {

    $delim = $node->getDelimiter();
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      if ($expressions_elem instanceof ScalarExpression) {
        $str = /*(string)*/$expressions_elem->getValue();
        if ($delim[0] === "'") {
          $str = $this->escapeForSingleQuoted($str);
        } else {
          $str = $this->escapeForDoubleQuoted($str);
        }
        $expressions[] = $str;
      } else {
        $expr = /*(string)*/$expressions_elem->accept($this);
        if (!$this->isSimpleEncapsExpr($expressions_elem)) {
          $expr = '{'.$expr.'}';
        }
        $expressions[] = $expr;
      }
    }

    switch ($delim) {
      case '`':
      case '"':
      case '\'':
        $open_delim = $delim;
        break;
      default:
        $expression_values = $node->getExpressions();
        $open_delim = '<<<'.$delim;
        if ($delim[0] === "'") {
          $delim = /*(string)*/substr($delim, 1, strlen($delim) - 2);
        }
        break;
    }
    return $this->visitVector($expressions, $open_delim, '', $delim,
      $open_delim.$delim);
  }

  /**
   *  from identifier in collection
   */
  public function visitFromClause(
    /*IFromClause*/ $node) /*: mixed*/ {
    $collection = /*(string)*/$node->getCollection()->accept($this);

    return 'from $'.$node->getIdentifier().' in '.$collection;
  }

  /**
   *  group collection by key
   */
  public function visitGroupClause(
    /*IGroupClause*/ $node) /*: mixed*/ {
    $collection = /*(string)*/$node->getCollection()->accept($this);
    $key = /*(string)*/$node->getKey()->accept($this);

    return 'group '.$collection.' by '.$key;
  }

  /**
   *  into identifier query_clauses
   */
  public function visitIntoClause(
    /*IIntoClause*/ $node) /*: mixed*/ {
    $clauses = Vector {};
    $this->indent .= '  ';
    foreach ($node->getClauses() as $clauses_elem) {
      $clauses[] = /*(string)*/$clauses_elem->accept($this);
    }

    $result = 'into $'.$node->getIdentifier();
    $result .= $this->visitVector($clauses,
      "\n".$this->indent, "\n".$this->indent, '');
    $this->indent = (string)substr($this->indent, 2);
    return $result;
  }

  /**
   *  join identifier in collection
   *    on left equals right [into group]
   */
  public function visitJoinClause(
    /*IJoinClause*/ $node) /*: mixed*/ {
    $collection = /*(string)*/$node->getCollection()->accept($this);
    $left = /*(string)*/$node->getLeft()->accept($this);
    $right = /*(string)*/$node->getRight()->accept($this);
    $group = $node->getGroup();

    $result = 'join $'.$node->getIdentifier().' in '.$collection.' on '
      .$left.' equals '.$right;
    if ($group !== null) {
      $result .= ' into '.$group;
    }
    return $result;
  }

  /**
   *  let identifier = expr
   */
  public function visitLetClause(
    /*ILetClause*/ $node) /*: mixed*/ {
    $expression = /*(string)*/$node->getExpression()->accept($this);

    return 'let $'.$node->getIdentifier().' = '.$expression;
  }

  /**
   *  list(variables) [ = expression ]
   */
  public function visitListAssignmentExpression(
    /*IListAssignmentExpression*/ $node) /*: mixed*/ {
    $variables = Vector {};
    foreach ($node->getVariables() as $variables_elem) {
      $variables[] = /*(string)*/(
        $variables_elem === null ? '' : $variables_elem->accept($this)
      );
    }
    $expression = null;
    $expression_value = $node->getExpression();
    if ($expression_value !== null) {
      $expression = /*(string)*/$expression_value->accept($this);
    }

    $result = $this->visitVector($variables, 'list(', ', ', ')', 'list()');
    if (is_string($expression)) { $result .= ' = '.$expression; }
    return $result;
  }

  /**
   *  new class(arguments)
   *  new {classExpression}(arguments)
   */
  public function visitNewObjectExpression(
    /*INewObjectExpression*/ $node) /*: mixed*/ {
    $class = null;
    $class_value = $node->getClass();
    if ($class_value !== null) {
      $class = /*(string)*/$class_value->accept($this);
    }
    $class_expression = null;
    $class_expression_value = $node->getClassExpression();
    if ($class_expression_value !== null) {
      $class_expression = /*(string)*/$class_expression_value->accept($this);
    }
    $arguments = Vector {};
    foreach ($node->getArguments() as $arguments_elem) {
      $arguments[] = /*(string)*/$arguments_elem->accept($this);
    }

    $result = 'new ';
    if (is_string($class)) {
      $result .= $class;
    } else if (is_string($class_expression)) {
      $result .= $class_expression;
    }
    $result .= $this->visitVector($arguments, '(', ', ', ')', '()');
    return $result;
  }

  /**
   * Wraps $expr in parenthesis if $expression is of kind that has
   * lower operator priority than the -> operator.
   */
  private function parenthesizeIfLowerThanArrow(
    /*IExpression*/ $expression, /*string*/ $expr) /*: string*/ {
    if ($expression instanceof BinaryOpExpression ||
        $expression instanceof ClosureExpression ||
        $expression instanceof ConditionalExpression ||
        $expression instanceof ListAssignmentExpression ||
        $expression instanceof NewObjectExpression ||
        $expression instanceof UnaryOpExpression) {
      $expr = '('.$expr.')';
    }
    return $expr;
  }

  /**
   *  object->methodName(arguments)
   *  object->{expression}(arguments)
   */
  public function visitObjectMethodCallExpression(
    /*IObjectMethodCallExpression*/ $node) /*: mixed*/ {
    $object_expression = $node->getObject();
    $object = /*(string)*/$object_expression->accept($this);
    $method_expression = null;
    $method_expression_value = $node->getMethodExpression();
    if ($method_expression_value !== null) {
      $method_expression = /*(string)*/$method_expression_value->accept($this);
    }
    $arguments = Vector {};
    foreach ($node->getArguments() as $arguments_elem) {
      $arguments[] = /*(string)*/$arguments_elem->accept($this);
    }

    $object = $this->parenthesizeIfLowerThanArrow($object_expression, $object);
    $result = $object.'->';
    if (is_string($method_expression)) {
      $result .= $method_expression;
    } else {
      $result .= $node->getMethodName();
    }
    $result .= $this->visitVector($arguments, '(', ', ', ')', '()');
    return $result;
  }

  /**
   *  object->propertyName
   *  object->{expression}
   */
  public function visitObjectPropertyExpression(
    /*IObjectPropertyExpression*/ $node) /*: mixed*/ {
    $object_expression = $node->getObject();
    $object = /*(string)*/$object_expression->accept($this);
    $property_expression = null;
    $property_expression_value = $node->getPropertyExpression();
    if ($property_expression_value !== null) {
      $property_expression =
        /*(string)*/$property_expression_value->accept($this);
    }

    $object = $this->parenthesizeIfLowerThanArrow($object_expression, $object);
    $result = $object.'->';
    if (is_string($property_expression)) {
      $result .= $property_expression;
    } else {
      $result .= $node->getPropertyName();
    }
    return $result;
  }

  /**
   *  orderby orderings
   */
  public function visitOrderbyClause(
    /*IOrderbyClause*/ $node) /*: mixed*/ {
    $orders = Vector {};
    foreach ($node->getOrders() as $orders_elem) {
      $orders[] = /*(string)*/$orders_elem->accept($this);
    }

    return 'orderby '.$this->visitVector($orders, '', ', ', '');
  }

  /**
   *  expression [order]
   */
  public function visitOrdering(
    /*IOrdering*/ $node) /*: mixed*/ {
    $expression = /*(string)*/$node->getExpression()->accept($this);

    $r = $expression;
    switch ($node->getOrder()) {
      case Orders::PHP_ASCENDING: $r .= ' ascending'; break;
      case Orders::PHP_DESCENDING: $r .= ' descending'; break;
    }
    return $r;
  }

  /**
   *  query_clauses
   */
  public function visitQueryExpression(
    /*IQueryExpression*/ $node) /*: mixed*/ {
    $clauses = Vector {};
    $this->indent .= '  ';
    foreach ($node->getClauses() as $clauses_elem) {
      $clauses[] = /*(string)*/$clauses_elem->accept($this);
    }

    $result = $this->visitVector($clauses,
     "\n".$this->indent, "\n".$this->indent, '');
    $this->indent = (string)substr($this->indent, 2);
    return $result;
  }

  private function escapeString(/*string*/ $str) /*: string*/ {
    $len = strlen($str);
    $sescape = false;
    $dquote = false;
    $result = '';
    for ($i = 0; $i < $len; $i++) {
      $c = $str[$i];
      switch ($c) {
        case "\n": $result .= '\\n'; $dquote = true; break;
        case "\r": $result .= '\\r'; $dquote = true; break;
        case "\t": $result .= '\\t'; $dquote = true; break;
        case "\v": $result .= '\\v'; $dquote = true; break;
        case "\e": $result .= '\\e'; $dquote = true; break;
        case "\f": $result .= '\\f'; $dquote = true; break;
        case '\\': $result .= '\\\\'; $sescape = true; break;
        case '$':  $result .= '\\$'; break;
        case '"':  $result .= '\\"'; break;
        case '\'': $result .= "'"; $sescape = true; break;
        default: {
          if ($c >= ' ' && $c <= '~') {
            // The range [' ', '~'] contains only printable characters
            $result .= /*(string)*/$c;
          } else {
            $dquote = true;
            $oct = /*(string)*/sprintf('%o', ord($c));
            $result .= '\\'.$oct;
          }
          break;
        }
      }
    }
    if (!$dquote) {
      $result = $str;
      if ($sescape) {
        $result = str_replace('\\', '\\\\', $result);
        $result = str_replace('\'', '\\\'', $result);
      }
      return '\''.$result.'\'';
    }
    return '"'.$result.'"';
  }

  /**
   *  a compile time constant of some type
   */
  public function visitScalarExpression(
    /*IScalarExpression*/ $node) /*: mixed*/ {

    $val = $node->getValue();
    if (is_string($val)) { return $this->escapeString($val); }
    $result = /*(string)*/$val;
    if (is_float($val) && !strpos($result, '.')) {
      $result .= '.0';
    }
    return $result;
  }

   /**
   *  select expression
   */
  public function visitSelectClause(
    /*ISelectClause*/ $node) /*: mixed*/ {
    $expression = /*(string)*/$node->getExpression()->accept($this);

    return 'select '.$expression;
  }

  /**
   *  name
   */
  public function visitSimpleConstantExpression(
    /*ISimpleConstantExpression*/ $node) /*: mixed*/ {
    return $node->getConstantName();
  }

 /**
   *  functionName(arguments)
   *  {functionExpression}(arguments)
   */
  public function visitSimpleFunctionCallExpression(
    /*ISimpleFunctionCallExpression*/ $node) /*: mixed*/ {
    $function_expression = null;
    $function_expression_value = $node->getFunctionExpression();
    if ($function_expression_value !== null) {
      $function_expression =
        /*(string)*/$function_expression_value->accept($this);
    }
    $arguments = Vector {};
    foreach ($node->getArguments() as $arguments_elem) {
      $arguments[] = /*(string)*/$arguments_elem->accept($this);
    }

    if (is_string($function_expression)) {
      $result = $function_expression;
    } else {
      $result = $node->getFunctionName();
    }
    $result .= $this->visitVector($arguments, '(', ', ', ')', '()');

    return $result;
  }

/**
 * $variableName
 * ${variableExpression}
 */
  public function visitSimpleVariableExpression(
    /*ISimpleVariableExpression*/ $node) /*: mixed*/ {
    $variable_expression = null;
    $variable_expression_value = $node->getVariableExpression();
    if ($variable_expression_value !== null) {
      $variable_expression =
        /*(string)*/$variable_expression_value->accept($this);
    }

    if (is_string($variable_expression)) {
      return '${'.$variable_expression.'}';
    } else {
      return '$'.$node->getVariableName();
    }
  }

  /**
   * A type expression results in a value of type Type.
   * But it's compile time type (getType()) is the runtime
   * type value (or null) rather than type Type.
   */
  public function visitTypeExpression(
    /*ITypeExpression*/ $node) /*: mixed*/ {
    $name = $node->getName();
    $type_arguments = Vector {};
    foreach ($node->getTypeArguments() as $type_arguments_elem) {
      $type_arguments[] = /*(string)*/$type_arguments_elem->accept($this);
    }
    $return_type = null;
    $return_type_value = $node->getReturnType();
    if ($return_type_value !== null) {
      $return_type = /*(string)*/$return_type_value->accept($this);
    }

    $result = '';
    if ($node->getIsNullable()) {
      $result = '?';
    }
    if ($node->getIsSoft()) {
      $result = '@'.$result;
    }
    if ($name === 'tuple') {
      $result .= $this->visitVector($type_arguments, '(', ', ', ')');
    } else if (is_string($return_type)) {
      $result .= '(function';
      $result .= $this->visitVector($type_arguments, '(', ', ', ')');
      $result .= ': '.$return_type.')';
    } else {
      $result .= $name;
      $result .= $this->visitVector($type_arguments, '<', ', ', '>', '');
    }
    return $result;
  }

  /**
   *   name [as constraint]
   */
  public function visitTypeParameterExpression(
    /*ITypeParameterExpression*/ $node) /*: mixed*/ {
    $constraint = null;
    $constraint_value = $node->getConstraint();
    if ($constraint_value !== null) {
      $constraint = /*(string)*/$constraint_value->accept($this);
    }

    $result = $node->getName();
    if (is_string($constraint)) {
      $result .= ' as '.$constraint;
    }
    return $result;
  }

  /**
   * Returns true the given postfix unary operation has a higher priority
   * than some operation in the given expression. In other words,
   * if (expr) op != expr op, return true.
   */
  private function postfixExprNeedsParentheses(
    /*UnaryOperator*/ $op, /*IExpression*/ $expr) /* : bool */ {
    if ($expr instanceof ConditionalExpression) {
      // let (expr) == (a ? b : c)
      // (a ? b : c) op != a ? b : c op in all cases
      return true;
    }
    if ($expr instanceof BinaryOpExpression) {
      // let (x) == (a bop b)
      // (a bop b) op != a bop b op if
      if (UnaryOperators::hasPriority($op, $expr->getOperation())) {
        return true;
      }
      // check if a bop (b) op == a bop b op
      return $this->postfixExprNeedsParentheses($op, $expr->getExpression2());
    }
    if ($expr instanceof UnaryOpExpression) {
      switch ($expr->getOperation()) {
        case UnaryOperators::PHP_ARRAY_APPEND_POINT_OP:
        case UnaryOperators::PHP_POST_DECREMENT_OP:
        case UnaryOperators::PHP_POST_INCREMENT_OP:
          // let (expr) == (a uop)
          // (a uop) op == a uop op in all cases
          return false;
      }
      // let (expr) == (uop a)
      // check if uop (a) op == uop a op
      return $this->postfixExprNeedsParentheses($op, $expr->getExpression());
    }
    return false;
  }

  /**
   * Returns true the given prefix unary operation has a higher priority
   * than some operation in the given expression. In other words,
   * if op (expr) != op expr, return true.
   */
  private function prefixExprNeedsParentheses(
    /*UnaryOperator*/ $op, /*IExpression*/ $expr) /* : bool */ {
    if ($expr instanceof ConditionalExpression) {
      // let (expr) == (a ? b : c)
      // op (a ? b : c) != op a ? b : c in all cases
      return true;
    }
    if ($expr instanceof BinaryOpExpression) {
      // let (expr) == (a bop b)
      // op (a bop b) != op a bop b if
      if (UnaryOperators::hasPriority($op, $expr->getOperation())) {
        return true;
      }
      // check if op (a) bop b == op a bop b
      return $this->prefixExprNeedsParentheses($op, $expr->getExpression1());
    }
    if ($expr instanceof UnaryOpExpression) {
      switch ($expr->getOperation()) {
        case UnaryOperators::PHP_ARRAY_APPEND_POINT_OP:
        case UnaryOperators::PHP_POST_DECREMENT_OP:
        case UnaryOperators::PHP_POST_INCREMENT_OP:
          // let (expr) == (a uop)
          // op (a uop) op != op a uop if
          return $op == UnaryOperators::PHP_CLONE_OP;
      }
      // let (expr) == (uop a)
      // op (uop a) == op uop a in all cases
      return false;
    }
    return false;
  }

  /**
   * Returns true the given unary operation has a higher priority
   * than some operation in the given expression. In other words,
   * if op (expr) != op expr or if (expr) op != expr op, return true.
   */
  private function exprNeedsParentheses(
    /*UnaryOperator*/ $op, /*IExpression*/ $expr) /* : bool */ {
    switch ($op) {
      case UnaryOperators::PHP_ARRAY_APPEND_POINT_OP:
      case UnaryOperators::PHP_POST_DECREMENT_OP:
      case UnaryOperators::PHP_POST_INCREMENT_OP:
        return $this->postfixExprNeedsParentheses($op, $expr);
    }
    return $this->prefixExprNeedsParentheses($op, $expr);
  }

  /**
   *  operation expression
   */
  public function visitUnaryOpExpression(
    /*IUnaryOpExpression*/ $node) /*: mixed*/ {
    $op = $node->getOperation();
    $expr = $node->getExpression();
    $expression = /*(string)*/$expr->accept($this);
    if ($this->exprNeedsParentheses($op, $expr)) {
      $expression = '('.$expression.')';
    }

    switch ($op) {
      case UnaryOperators::PHP_ARRAY_CAST_OP:
        return '(array)'.$expression;
      case UnaryOperators::PHP_ARRAY_APPEND_POINT_OP:
        return $expression.'[]';
      case UnaryOperators::PHP_AWAIT_OP:
        return 'await '.$expression;
      case UnaryOperators::PHP_BOOL_CAST_OP:
        return '(bool)'.$expression;
      case UnaryOperators::PHP_BITWISE_NOT_OP:
        return '~'.$expression;
      case UnaryOperators::PHP_CLONE_OP:
        return 'clone '.$expression;
      case UnaryOperators::PHP_DYNAMIC_VARIABLE_OP:
        return '$'.$expression;
      case UnaryOperators::PHP_ERROR_CONTROL_OP:
        return '@'.$expression;
      case UnaryOperators::PHP_FLOAT_CAST_OP:
        return '(float)'.$expression;
      case UnaryOperators::PHP_INCLUDE_OP:
        return 'include '.$expression;
      case UnaryOperators::PHP_INCLUDE_ONCE_OP:
        return 'include_once '.$expression;
      case UnaryOperators::PHP_INT_CAST_OP:
        return '(int)'.$expression;
      case UnaryOperators::PHP_MINUS_OP:
        return '-'.$expression;
      case UnaryOperators::PHP_NOT_OP:
        return '!'.$expression;
      case UnaryOperators::PHP_OBJECT_CAST_OP:
        return '(object)'.$expression;
      case UnaryOperators::PHP_PLUS_OP:
        return '+'.$expression;
      case UnaryOperators::PHP_POST_DECREMENT_OP:
        return $expression.'--';
      case UnaryOperators::PHP_POST_INCREMENT_OP:
        return $expression.'++';
      case UnaryOperators::PHP_PRE_DECREMENT_OP:
        return '--'.$expression;
      case UnaryOperators::PHP_PRE_INCREMENT_OP:
        return '++'.$expression;
      case UnaryOperators::PHP_PRINT_OP:
        return 'print '.$expression;
      case UnaryOperators::PHP_REFERENCE_OP:
        return '&'.$expression;
      case UnaryOperators::PHP_REQUIRE_OP:
        return 'require '.$expression;
      case UnaryOperators::PHP_REQUIRE_ONCE_OP:
        return 'require_once '.$expression;
      case UnaryOperators::PHP_STRING_CAST_OP:
        return '/*(string)*/'.$expression;
      case UnaryOperators::PHP_UNSET_CAST_OP:
        return '(unset)'.$expression;
    }
    return 'invalid '.$expression;
  }

  /**
   *  where condition
   */
  public function visitWhereClause(
    /*IWhereClause*/ $node) /*: mixed*/ {
    $condition = /*(string)*/$node->getCondition()->accept($this);

    return 'where '.$condition;
  }

  /**
   *  <tag attr=expression, ... > {expression} <tag/>
   */
  public function visitXmlExpression(
    /*IXmlExpression*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $elements = Vector {};
    foreach ($node->getElements() as $elements_elem) {
      $elements[] = /*(string)*/$elements_elem->accept($this);
    }

    $result = '<'.$node->getTagName();
    $result .= $this->visitVector($attributes, ' ', ', ', '>', '>');
    $result .= $this->visitVector($elements, '', '', '');
    $result .= '<'.$node->getTagName().'/>';
    return $result;
  }

  /**
   *  yield [key =>] value
   */
  public function visitYieldExpression(
    /*IYieldExpression*/ $node) /*: mixed*/ {
    $key = null;
    $key_value = $node->getKey();
    if ($key_value !== null) {
      $key = /*(string)*/$key_value->accept($this);
    }
    $value = /*(string)*/$node->getValue()->accept($this);

    $result = 'yield ';
    if (is_string($key_value)) {
      $result .= $key_value.' => ';
    }
    return $result.$value;
  }

  /**
   *  { statements }
   */
  public function visitBlockStatement(
    /*IBlockStatement*/ $node) /*: mixed*/ {
    $is_enclosed = $node->getIsEnclosed();
    if ($is_enclosed) {
      $result = $this->blockIndent.'{';
    } else {
      $result = '';
    }
    $this->indent .= '  ';
    $this->blockIndent = $this->indent;
    $statements = Vector {};
    foreach ($node->getStatements() as $statements_elem) {
      $statements[] = /*(string)*/$statements_elem->accept($this);
    }
    $this->indent = /*(string)*/substr($this->indent, 2);
    $this->blockIndent = $this->indent;

    $result .= $this->visitVector($statements, "\n", '', '');
    if ($is_enclosed) {
      $result .= $this->indent.'}';
    } else if (substr($result, -1) === "\n") {
      $result = substr($result, 0, -1);
    }
    return $result;
  }

  /**
   *  break [depth];
   */
  public function visitBreakStatement(
    /*IBreakStatement*/ $node) /*: mixed*/ {

    $depth = $node->getDepth();
    if ($depth == 1) {
      return $this->indent."break;\n";
    } else {
      return $this->indent."break $depth;\n";
    }
  }

  /**
   *  case condition: block
   *  default: block
   */
  public function visitCaseStatement(
    /*ICaseStatement*/ $node) /*: mixed*/ {
    $condition = null;
    $condition_value = $node->getCondition();
    if ($condition_value !== null) {
      $condition = /*(string)*/$condition_value->accept($this);
    }
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    if (is_string($condition)) {
      return $this->indent.'case '.$condition.':'.$block."\n";
    } else {
      return $this->indent.'default:'.$block."\n";
    }
  }

  /**
   *  catch (class variableName) block      (no newline at end of block)
   */
  public function visitCatchStatement(
    /*ICatchStatement*/ $node) /*: mixed*/ {

    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);
    $class = /*(string)*/$node->getClass()->accept($this);
    $variable_name = $node->getVariableName();

    return " catch ($class \$$variable_name)".$block;
  }

  /**
   *  modifiers [typeAnnotation] expressions;
   */
  public function visitClassVariableStatement(
    /*IClassVariableStatement*/ $node) /*: mixed*/ {
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $type_annotation = null;
    $type_annotation_value = $node->getTypeAnnotation();
    if ($type_annotation_value !== null) {
      $type_annotation = /*(string)*/$type_annotation_value->accept($this);
    }
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $result = "\n".$this->indent;
    $result .= $this->visitVector($modifiers, '', ' ', ' ');
    if (is_string($type_annotation)) {
      $result .= $type_annotation.' ';
    }
    $result .= $this->visitVector($expressions, '', ', ', '');
    return $result.";\n";
  }

  /**
   *  if (condition) trueBlock else falseBlock
   */
  public function visitConditionalStatement(
    /*IConditionalStatement*/ $node) /*: mixed*/ {
    $condition = /*(string)*/$node->getCondition()->accept($this);
    $this->blockIndent = ' ';
    $true_block = /*(string)*/$node->getTrueBlock()->accept($this);
    $this->blockIndent = ' ';
    $false_block = /*(string)*/$node->getFalseBlock()->accept($this);
    $result = $this->indent.'if ('.$condition.')'.$true_block;
    if ($false_block !== '') {
      $result .= ' else'.$false_block;
    }
    return $result."\n";
  }

  /**
   *  const [typeAnnotation] expressions;
   */
  public function visitConstantStatement(
    /*IConstantStatement*/ $node) /*: mixed*/ {
    $type_annotation = null;
    $type_annotation_value = $node->getTypeAnnotation();
    if ($type_annotation_value !== null) {
      $type_annotation = /*(string)*/$type_annotation_value->accept($this);
    }
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $result = "\n".$this->indent.'const ';
    if (is_string($type_annotation)) {
      $result .= $type_annotation.' ';
    }
    $result .= $this->visitVector($expressions, '', ', ', '');
    return $result.";\n";
  }

  /**
   *  continue depth;
   */
  public function visitContinueStatement(
    /*IContinueStatement*/ $node) /*: mixed*/ {

    $depth = $node->getDepth();
    if ($depth == 1) {
      return $this->indent."continue;\n";
    } else {
      return $this->indent."continue $depth;\n";
    }
  }

  /**
   *  do block while (condition);
   */
  public function visitDoStatement(
    /*IDoStatement*/ $node) /*: mixed*/ {
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);
    $condition = /*(string)*/$node->getCondition()->accept($this);

    $result = $this->indent.'do'.$block;
    $result .= ' while ('.$condition.");\n";
    return $result;
  }

  /**
   *  echo expressions;
   */
  public function visitEchoStatement(
    /*IEchoStatement*/ $node) /*: mixed*/ {
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $exprs = $this->visitVector($expressions, '', ', ', '');
    return $this->indent.'echo '.$exprs.";\n";
  }

  /**
   *  expression;
   */
  public function visitExpressionStatement(
    /*IExpressionStatement*/ $node) /*: mixed*/ {
    $expression = /*(string)*/$node->getExpression()->accept($this);

    return $this->indent.$expression.";\n";
  }

  /**
   *  finally block   (no newline at end of block)
   */
  public function visitFinallyStatement(
    /*IFinallyStatement*/ $node) /*: mixed*/ {
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    return ' finally'.$block;
  }

  /**
   *  for (initializers; conditions; increments) block
   */
  public function visitForStatement(
    /*IForStatement*/ $node) /*: mixed*/ {
    $initializers = Vector {};
    foreach ($node->getInitializers() as $initializers_elem) {
      $initializers[] = /*(string)*/$initializers_elem->accept($this);
    }
    $conditions = Vector {};
    foreach ($node->getConditions() as $conditions_elem) {
      $conditions[] = /*(string)*/$conditions_elem->accept($this);
    }
    $increments = Vector {};
    foreach ($node->getIncrements() as $increments_elem) {
      $increments[] = /*(string)*/$increments_elem->accept($this);
    }
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    $result = $this->indent.'for (';
    $result .= $this->visitVector($initializers, '', ',', '; ', '; ');
    $result .= $this->visitVector($conditions, '', ',', '; ', '; ');
    $result .= $this->visitVector($increments, '', ',', '', '');
    $result .= ')'.$block."\n";
    return $result;
  }

  /**
   *  foreach (collection as [key =>] value) block
   */
  public function visitForeachStatement(
    /*IForeachStatement*/ $node) /*: mixed*/ {
    $collection = /*(string)*/$node->getCollection()->accept($this);
    $key = null;
    $key_value = $node->getKey();
    if ($key_value !== null) {
      $key = /*(string)*/$key_value->accept($this);
    }
    $value = /*(string)*/$node->getValue()->accept($this);
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    $result = $this->indent.'foreach (';
    $result .= $collection.' as ';
    if (is_string($key)) {
      $result .= $key.' => ';
    }
    $result .= $value.')'.$block."\n";
    return $result;
  }

  /**
   *  [ << attributes >> ]
   *  [modifiers] function [&] name [<typeParameters>]
   *  (parameters) [: returnType]
   *    block
   */
  public function visitFunctionStatement(
    /*IFunctionStatement*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $type_parameters = Vector {};
    foreach ($node->getTypeParameters() as $type_parameters_elem) {
      $type_parameters[] = /*(string)*/$type_parameters_elem->accept($this);
    }
    $parameters = Vector {};
    foreach ($node->getParameters() as $parameters_elem) {
      $parameters[] = /*(string)*/$parameters_elem->accept($this);
    }
    $return_type = null;
    $return_type_value = $node->getReturnType();
    if ($return_type_value !== null) {
      $return_type = /*(string)*/$return_type_value->accept($this);
    }
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    $result = "\n".$this->indent;
    $result .= $this->visitVector($attributes, '<<', ', ',
      ">>\n".$this->indent, '');
    $result .= $this->visitVector($modifiers, '', ' ', ' ', '');
    $result .= 'function ';
    if ($node->getReturnsReference()) { $result .= '&'; }
    $result .= $node->getName();
    $result .= $this->visitVector($type_parameters, '<', ', ', '>', '');
    $result .= $this->visitVector($parameters, '(', ', ', ')', '()');
    if (is_string($return_type)) {
      $result .= ' : '.$return_type;
    }
    if ($block === '') {
      return $result.";\n";
    } else {
      return $result.$block."\n";
    }
  }

  /**
   *  global expressions;
   */
  public function visitGlobalStatement(
    /*IGlobalStatement*/ $node) /*: mixed*/ {
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $result = $this->indent.'global ';
    $result .= $this->visitVector($expressions, '', ', ', '');
    return $result.";\n";
  }

  /**
   *  goto label;
   */
  public function visitGotoStatement(
    /*IGotoStatement*/ $node) /*: mixed*/ {

    return $this->indent.'goto '.$node->getLabel().";\n";
  }

  /**
   *  label:
   */
  public function visitLabelStatement(
    /*ILabelStatement*/ $node) /*: mixed*/ {
    return $this->indent.$node->getLabel().":\n";
  }

  /**
   *  [ << attributes >> ]
   *  [modifiers] [typeAnnotation] [&] name [= expression];
   */
  public function visitParameterDeclaration(
    /*IParameterDeclaration*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $type_annotation = null;
    $type_annotation_value = $node->getTypeAnnotation();
    if ($type_annotation_value !== null) {
      $type_annotation = /*(string)*/$type_annotation_value->accept($this);
    }
    $expression = null;
    $expression_value = $node->getExpression();
    if ($expression_value !== null) {
      $expression = /*(string)*/$expression_value->accept($this);
    }

    $result = $this->visitVector($attributes, '<< ', ' ', ' >> ', '');
    $result .= $this->visitVector($modifiers, '', ' ', ' ', '');
    if (is_string($type_annotation)) {
      $result .= $type_annotation.' ';
    }
    $result .= '$'.$node->getName();
    if (is_string($expression)) {
      $result .= ' = '.$expression;
    }
    return $result;
  }

  /**
   *  return [expression];
   */
  public function visitReturnStatement(
    /*IReturnStatement*/ $node) /*: mixed*/ {
    $expression = null;
    $expression_value = $node->getExpression();
    if ($expression_value !== null) {
      $expression = /*(string)*/$expression_value->accept($this);
    }

    $result = $this->indent.'return';
    if (is_string($expression)) {
      $result .= ' '.$expression;
    }
    return $result.";\n";
  }

  /**
   *  static expressions;
   */
  public function visitStaticStatement(
    /*IStaticStatement*/ $node) /*: mixed*/ {
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $result = $this->indent.'static ';
    $result .= $this->visitVector($expressions, '', ', ', '');
    return $result.";\n";
  }

  /**
   *  switch (expression) { caseStatements }
   */
  public function visitSwitchStatement(
    /*ISwitchStatement*/ $node) /*: mixed*/ {
    $expression = /*(string)*/$node->getExpression()->accept($this);
    $this->indent .= '  ';
    $case_statements = Vector {};
    foreach ($node->getCaseStatements() as $case_statements_elem) {
      $case_statements[] = /*(string)*/$case_statements_elem->accept($this);
    }
    $this->indent = /*(string)*/substr($this->indent, 2);

    $result = $this->indent.'switch ('.$expression.") {\n";
    $result .= $this->visitVector($case_statements, '', '', '');
    $result .= $this->indent."}\n";
    return $result;
  }

  /**
   *  throw expression;
   */
  public function visitThrowStatement(
    /*IThrowStatement*/ $node) /*: mixed*/ {
    $expression = /*(string)*/$node->getExpression()->accept($this);

    return $this->indent.'throw '.$expression.";\n";
  }

  /**
   *  [traitName::]methodName1 as modifiers methodName2;
   */
  public function visitTraitAliasStatement(
    /*ITraitAliasStatement*/ $node) /*: mixed*/ {
    $trait_name = $node->getTraitName();
    $method_name1 = $node->getMethodName1();
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $method_name2 = $node->getMethodName2();

    $result = $this->indent;
    if (is_string($trait_name)) {
      $result .= $trait_name.'::';
    }
    $result .= $method_name1.' as ';
    $result .= $this->visitVector($modifiers, '', ' ', '');
    $result .= $method_name2.";\n";
    return $result;
  }

  /**
   *  traitName::methodName instead otherTraitNames;
   */
  public function visitTraitInsteadStatement(
    /*ITraitInsteadStatement*/ $node) /*: mixed*/ {
    $trait_name = $node->getTraitName();
    $method_name = $node->getMethodName();
    $other_trait_names = $node->getOtherTraitNames();

    $result = $this->indent.$trait_name.'::';
    $result .= $method_name.' instead ';
    $result .= $this->visitVector($other_trait_names, '', ' ', '');
    $result .= ";\n";
    return $result;
  }

  /**
   *  require extends|implements name;
   */
  public function visitTraitRequireExtendsStatement(
    /*ITraitRequiresStatement*/ $node) /*: mixed*/ {

    $result = $this->indent.'require ';
    switch ($node->getKind()) {
      case RequireKinds::PHP_EXTENDS: $result .= 'extends '; break;
      case RequireKinds::PHP_IMPLEMENTS: $result .= 'implements '; break;
    }
    return $result.$node->getName();
  }

  /**
   *  try block catchStatements [finallyStatement]
   */
  public function visitTryStatement(
    /*ITryStatement*/ $node) /*: mixed*/ {
    $this->blockIndent = ' ';
    $block = /*(string)*/rtrim($node->getBlock()->accept($this));
    $catch_statements = Vector {};
    foreach ($node->getCatchStatements() as $catch_statements_elem) {
      $catch_statements[] = /*(string)*/$catch_statements_elem->accept($this);
    }
    $finally_statement = null;
    $finally_statement_value = $node->getFinallyStatement();
    if ($finally_statement_value !== null) {
      $finally_statement = /*(string)*/$finally_statement_value->accept($this);
    }

    $result = $this->indent.'try '.$block;
    $result .= $this->visitVector($catch_statements, '', '', '');
    return $result."\n";
  }

  /**
   * [ << attributes >> ]
   * [modifiers] class|interface|trait name [typeParameters] [extends baseClass]
   * [implements interfaces]
   *   block
   */
  public function visitTypeStatement(
    /*ITypeStatement*/ $node) /*: mixed*/ {
    $attributes = Vector {};
    foreach ($node->getAttributes() as $attributes_elem) {
      $attributes[] = /*(string)*/$attributes_elem->accept($this);
    }
    $modifiers = Vector {};
    foreach ($node->getModifiers() as $modifiers_elem) {
      $modifiers[] = /*(string)*/$modifiers_elem->accept($this);
    }
    $type_parameters = Vector {};
    foreach ($node->getTypeParameters() as $type_parameters_elem) {
      $type_parameters[] = /*(string)*/$type_parameters_elem->accept($this);
    }
    $base_class = null;
    $base_class_value = $node->getBaseClass();
    if ($base_class_value !== null) {
      $base_class = /*(string)*/$base_class_value->accept($this);
    }
    $interfaces = Vector {};
    foreach ($node->getInterfaces() as $interfaces_elem) {
      $interfaces[] = /*(string)*/$interfaces_elem->accept($this);
    }
    $this->blockIndent = '';
    $block = /*(string)*/$node->getBlock()->accept($this);
    $this->blockIndent = '';

    $result = $this->indent;
    $result .= $this->visitVector($attributes, '<<', ', ',
      ">>\n".$this->indent, '');
    $result .= $this->visitVector($modifiers, '', ' ', ' ', '');
    switch ($node->getKind()) {
      case TypeKinds::PHP_CLASS: $result .= 'class '; break;
      case TypeKinds::PHP_INTERFACE: $result .= 'interface '; break;
      case TypeKinds::PHP_TRAIT: $result .= 'trait '; break;
    }
    $result .= $node->getName();
    $result .= $this->visitVector($type_parameters, '<', ', ', '>', '');
    $result .= ' ';
    if (is_string($base_class)) {
      $result .= 'extends '.$base_class.' ';
    }
    $result .= $this->visitVector($interfaces, 'implements ', ', ', ' ', '');
    $result .= $block."\n\n";
    return $result;
  }

  /**
   *  type name [typeParameters] [as constraint] = typeAnnotation;
   *  newtype name [typeParameters] [as constraint] = typeAnnotation;
   */
  public function visitTypedefStatement(
    /*ITypedefStatement*/ $node) /*: mixed*/ {
    $type_parameters = Vector {};
    foreach ($node->getTypeParameters() as $type_parameters_elem) {
      $type_parameters[] = /*(string)*/$type_parameters_elem->accept($this);
    }
    $constraint = null;
    $constraint_value = $node->getConstraint();
    if ($constraint_value !== null) {
      $constraint = /*(string)*/$constraint_value->accept($this);
    }
    $type_annotation = /*(string)*/$node->getTypeAnnotation()->accept($this);

    $result = $this->indent.($node->getIsOpaque() ? 'newtype' : 'type');
    $result .= $this->visitVector($type_parameters, '<', ', ', '>', '');
    if (is_string($constraint)) {
      $result .= ' as '.$constraint;
    }
    $result .= ' = '.$type_annotation.";\n";
    return $result;
  }

  /**
   * unset(expressions);
   */
  public function visitUnsetStatement(
    /*IUnsetStatement*/ $node) /*: mixed*/ {
    $expressions = Vector {};
    foreach ($node->getExpressions() as $expressions_elem) {
      $expressions[] = /*(string)*/$expressions_elem->accept($this);
    }

    $result = $this->indent.'unset';
    $result .= $this->visitVector($expressions, '(', ',', ')', '()');
    return $result.";\n";
  }

  /**
   *  use usedNamespaces;
   */
  public function visitUseNamespacesStatement(
    /*IUseNamespacesStatement*/ $node) /*: mixed*/ {
    $used_namespaces = Vector {};
    foreach ($node->getUsedNamespaces() as $used_namespaces_elem) {
      $used_namespaces[] =
        /*(string)*/substr($used_namespaces_elem->accept($this), 4, -2);
    }

    $result = $this->indent.'use ';
    $result .= $this->visitVector($used_namespaces, ' ', ', ', '');
    return $result.";\n";
  }

  /**
   *  use typeExpressions block
   */
  public function visitUseTraitStatement(
    /*IUseTraitStatement*/ $node) /*: mixed*/ {
    $expressions = Vector {};
    foreach ($node->getTypeExpressions() as $type_expressions_elem) {
      $expressions[] = /*(string)*/$type_expressions_elem->accept($this);
    }
    $this->blockIndent = ' ';
    $block = /*(string)*/$node->getBlock()->accept($this);

    $result = $this->indent.'use';
    $result .= $this->visitVector($expressions, ' ', ', ', '', '');
    if ($block === '') { $block = ';'; }
    $result .= $block."\n";
    return $result;
  }

  /**
   *  use namespace [as alias];
   */
  public function visitUsedNamespaceStatement(
    /*IUsedNamespaceStatement*/ $node) /*: mixed*/ {
    $namespace_name = $node->getNamespaceName();
    $alias_name = $node->getAliasName();

    $result = $this->indent.'use '.$namespace_name;
    if (is_string($alias_name)) {
      $result .= ' as '.$alias_name;
    }
    return $result.";\n";
  }

  /**
   *  while (condition) block
   */
  public function visitWhileStatement(
    /*IWhileStatement*/ $node) /*: mixed*/ {
    $this->blockIndent = ' ';
    $block = /*(string)*/rtrim($node->getBlock()->accept($this));
    $condition = /*(string)*/$node->getCondition()->accept($this);

    return $this->indent.'while ('.$condition.')'.$block."\n";
  }

  /*
   * typeName [ <typearg, ...> ]
   */
  public function visitTypeDataAsReference(
    /*ITypeData*/ $node) /*: mixed*/ {

    $type_arguments = Vector {};
    foreach ($node->getTypeArguments() as $type_arguments_elem) {
      $type_arguments[] =
        /*(string)*/$type_arguments_elem->acceptAsReference($this);
    }

    $result = $node->getName();
    $result = $this->visitVector($type_arguments, '<', ', ', '>', '');
    return $result;
  }
}
}
