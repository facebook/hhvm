<?hh

/**
 * An example DSL for testing expression trees (ETs).
 *
 * Any class can be used an an expression tree visitor. It needs to implement
 * the methods shown here, and expression tree literals MyClass`...` will be
 * converted (at compile time) to calls on these methods.
 */

// main function
function print_et<TInfer>(
  ExprTree<ExampleDsl, ExampleDsl::TAst, TInfer> $et,
): void {
  $visitor = new ExampleDsl();
  $string = $et->visit($visitor);
  echo($string."\n");
}

// The type checker requires expression trees to implement this interface specifying
// the type of the DSL/visitor, the type that the visitor computes, and the virtual type
// that the expression tree will produce.
interface Spliceable<TVisitor, +TResult, +TInfer> {
  public function visit(TVisitor $v): TResult;
}

type ExprTreeInfo<TInfer> = shape(
  'splices' => dict<string, mixed>,
  'functions' => vec<mixed>,
  'static_methods' => vec<mixed>,
  // The virtualised expression is placed here, to cause the type checker to instantiate
  // TInfer to the virtualised type.
  ?'type' => (function(): TInfer),
);

final class ExprTree<TVisitor, TResult, +TInfer>
  implements Spliceable<TVisitor, TResult, TInfer> {
  public function __construct(
    private ?ExprPos $pos,
    private ExprTreeInfo<TInfer> $metadata,
    private (function(TVisitor): TResult) $ast,
  )[] {}

  public function visit(TVisitor $v): TResult {
    return ($this->ast)($v);
  }

  public function getExprPos(): ?ExprPos {
    return $this->pos;
  }

  public function getSplices(): dict<string, mixed> {
    return $this->metadata['splices'];
  }
}

// The DSL itself: used as in ExampleDsl`...`. hackc generates a call to makeTree, which
// should return something that implements Spliceable, here an ExprTree
class ExampleDsl {
  const type TAst = string;

  // The desugared expression tree literal will call this method.
  public static function makeTree<TInfer>(
    ?ExprPos $pos,
    shape(
      'splices' => dict<string, mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>,
      ?'type' => (function(): TInfer),
    ) $metadata,
    (function(ExampleDsl): ExampleDsl::TAst) $ast,
  )[]: ExprTree<ExampleDsl, ExampleDsl::TAst, TInfer> {
    return new ExprTree($pos, $metadata, $ast);
  }

  // Virtual types. These do not have to be implemented, as they are only used
  // in the virtualized version of the expression tree, to work out the virtual type
  // of literals during type checking.
  public static function intType()[]: ExampleInt {
    throw new Exception();
  }
  public static function floatType(): ExampleFloat {
    throw new Exception();
  }
  public static function boolType(): ExampleBool {
    throw new Exception();
  }
  public static function stringType(): ExampleString {
    throw new Exception();
  }
  public static function nullType(): null {
    throw new Exception();
  }
  public static function voidType(): ExampleVoid {
    throw new Exception();
  }
  public static function symbolType<T>(
    (function(
      ExampleContext,
    ): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, T>>) $_,
  ): T {
    throw new Exception();
  }
  public static function lambdaType<T>(T $_): ExampleFunction<T> {
    throw new Exception();
  }
  public static function shapeType<T>(T $_): ExampleShape<T> {
    throw new Exception();
  }

  // Desugared nodes. Calls to these are emitted by hackc, following the structure
  // of the expression in the expression tree. Here, they compute a string
  // representation of that expression.
  public function visitInt(?ExprPos $_, int $i)[]: ExampleDsl::TAst {
    return (string)$i;
  }

  public function visitFloat(?ExprPos $_, float $f)[]: ExampleDsl::TAst {
    return (string)$f;
  }

  public function visitBool(?ExprPos $_, bool $b)[]: ExampleDsl::TAst {
    return $b ? "true" : "false";
  }

  public function visitString(?ExprPos $_, string $s)[]: ExampleDsl::TAst {
    return "\"$s\"";
  }

  public function visitNull(?ExprPos $_)[]: ExampleDsl::TAst {
    return "null";
  }

  // Expressions
  public function visitBinop(
    ?ExprPos $_,
    ExampleDsl::TAst $lhs,
    string $op,
    ExampleDsl::TAst $rhs,
  )[]: ExampleDsl::TAst {
    return "$lhs $op $rhs";
  }

  public function visitUnop(
    ?ExprPos $_,
    string $operator,
    ExampleDsl::TAst $operand,
  )[]: ExampleDsl::TAst {
    return "$operator $operand";
  }

  public function visitKeyedCollection(
    ?ExprPos $_,
    string $collection,
    (ExampleDsl::TAst, ExampleDsl::TAst) ...$operand
  )[]: ExampleDsl::TAst {
    $v = HH\Lib\Vec\map($operand, $kv ==> $kv[0]."=>".$kv[1]);
    return "$collection {".concat_arg_list($v)."}";
  }

  public function visitLocal(?ExprPos $_, string $local)[]: ExampleDsl::TAst {
    return $local;
  }

  public function visitLambda(
    ?ExprPos $_,
    vec<string> $args,
    vec<ExampleDsl::TAst> $body,
    vec<ExampleDsl::TAst> $args_with_optional_params = vec[],
  )[]: ExampleDsl::TAst {
    return "(".concat_arg_list(HH\Lib\Vec\concat($args, $args_with_optional_params)).")"." ==> {\n".concat_block($body)."}";
  }

  public function visitOptionalParameter(?ExprPos $_, string $param_name, ExampleDsl::TAst $default_value)[]: ExampleDsl::TAst {
    return $param_name . " = ". $default_value;
  }

  public function visitGlobalFunction<T>(
    ?ExprPos $_,
    (function(
      ExampleContext,
    ): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, T>>) $_,
  )[]: ExampleDsl::TAst {
    return "function_ptr";
  }

  public function visitStaticMethod<T>(
    ?ExprPos $_,
    (function(
      ExampleContext,
    ): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, T>>) $_,
  )[]: ExampleDsl::TAst {
    return "function_ptr";
  }

  // Operators
  public function visitCall(
    ?ExprPos $_,
    ExampleDsl::TAst $callee,
    vec<ExampleDsl::TAst> $args,
  )[]: ExampleDsl::TAst {
    return $callee."(".concat_arg_list($args).")";
  }

  public function visitAssign(
    ?ExprPos $_,
    ExampleDsl::TAst $local,
    ExampleDsl::TAst $value,
  )[]: ExampleDsl::TAst {
    return $local." = ".$value.";";
  }

  public function visitTernary(
    ?ExprPos $_,
    ExampleDsl::TAst $condition,
    ?ExampleDsl::TAst $truthy,
    ExampleDsl::TAst $falsy,
  )[]: ExampleDsl::TAst {
    return $condition." ? ".$truthy." : ".$falsy;
  }

  // Statements.
  public function visitIf(
    ?ExprPos $_,
    ExampleDsl::TAst $cond,
    vec<ExampleDsl::TAst> $then_body,
    vec<ExampleDsl::TAst> $else_body,
  )[]: ExampleDsl::TAst {
    return "if (".
      $cond.
      ") {\n".
      concat_block($then_body).
      "} else {\n".
      concat_block($else_body).
      "}";
  }

  public function visitWhile(
    ?ExprPos $_,
    ExampleDsl::TAst $cond,
    vec<ExampleDsl::TAst> $body,
  )[]: ExampleDsl::TAst {
    return "while (".$cond.") {\n".concat_block($body)."}";
  }

  public function visitReturn(
    ?ExprPos $_,
    ?ExampleDsl::TAst $return_val,
  )[]: ExampleDsl::TAst {
    if ($return_val is null) {
      return "return;";
    }
    return "return ".$return_val.";";
  }

  public function visitFor(
    ?ExprPos $_,
    vec<ExampleDsl::TAst> $initializers,
    ?ExampleDsl::TAst $cond,
    vec<ExampleDsl::TAst> $increments,
    vec<ExampleDsl::TAst> $body,
  )[]: ExampleDsl::TAst {
    return "for (".
      concat_arg_list($initializers).
      ";".
      ($cond ?? "").
      ";".
      concat_arg_list($increments).
      ") {\n".
      concat_block($body).
      "}";
  }

  public function visitBreak(?ExprPos $_)[]: ExampleDsl::TAst {
    return "break;";
  }

  public function visitContinue(?ExprPos $_)[]: ExampleDsl::TAst {
    return "continue;";
  }

  public function visitPropertyAccess(
    ?ExprPos $_,
    ExampleDsl::TAst $expr,
    string $prop_name,
  )[]: ExampleDsl::TAst {
    return "$expr->$prop_name";
  }

  public function visitInstanceMethod(
    ?ExprPos $_,
    ExampleDsl::TAst $expr,
    string $method,
  ): ExampleDsl::TAst {
    return "$expr->$method";
  }

  public function visitXhp(
    ?ExprPos $_,
    string $class_name,
    dict<string, ExampleDsl::TAst> $attrs,
    vec<ExampleDsl::TAst> $children,
  )[]: ExampleDsl::TAst {
    $attr_parts = vec[];
    foreach ($attrs as $name => $value) {
      $attr_parts[] = $name."=".$value;
    }
    $attr_string = implode(" ", $attr_parts);
    return "<".
      $class_name.
      " ".
      $attr_string.
      ">\n".
      concat_block($children).
      "\n</".
      $class_name.
      ">";
  }

  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    ExampleDslExpression<T> $splice_val,
  ): ExampleDsl::TAst {
    return "\${".($splice_val->visit($this))."}";
  }

  public function macroSplice<T>(
    ?ExprPos $_,
    string $_key,
    (function (): ExampleDslExpression<T>) $splice_val,
    vec<string> $macro_vars,
  ): ExampleDsl::TAst {
    return "\${/* ".concat_arg_list($macro_vars)." */".($splice_val()->visit($this))."}";
  }

 public function asyncMacroSplice<T>(
    ?ExprPos $_,
    string $_key,
    (function (): Awaitable<ExampleDslExpression<T>>) $splice_val,
    vec<string> $macro_vars,
  ): ExampleDsl::TAst {
    return "\${/* async ".concat_arg_list($macro_vars)." */ <async function>}";
  }

  public function visitShape(
    ?ExprPos $_,
    vec<(ExampleDsl::TAst, ExampleDsl::TAst)> $operand
  )[]: ExampleDsl::TAst {
    $v = HH\Lib\Vec\map($operand, $kv ==> $kv[0]."=>".$kv[1]);
    return "shape(".concat_arg_list($v).")";
  }
}

// Type declarations used when checking DSL expressions.
interface ExampleMixed {
  public function __tripleEquals(ExampleMixed $_): ExampleBool;
  public function __notTripleEquals(ExampleMixed $_): ExampleBool;
}

interface ExampleArraykey extends ExampleMixed {}

interface ExampleInt extends ExampleArraykey {
  public function __plus(ExampleInt $_): ExampleInt;
  public function __minus(ExampleInt $_): ExampleInt;
  public function __star(ExampleInt $_): ExampleInt;
  public function __slash(ExampleInt $_): ExampleInt;
  public function __percent(ExampleInt $_): ExampleInt;
  public function __negate(): ExampleInt;

  public function __lessThan(ExampleInt $_): ExampleBool;
  public function __lessThanEqual(ExampleInt $_): ExampleBool;
  public function __greaterThan(ExampleInt $_): ExampleBool;
  public function __greaterThanEqual(ExampleInt $_): ExampleBool;

  public function __amp(ExampleInt $_): ExampleInt;
  public function __bar(ExampleInt $_): ExampleInt;
  public function __caret(ExampleInt $_): ExampleInt;
  public function __lessThanLessThan(ExampleInt $_): ExampleInt;
  public function __greaterThanGreaterThan(ExampleInt $_): ExampleInt;
  public function __tilde(): ExampleInt;
  public function __postfixPlusPlus(): void;
  public function __postfixMinusMinus(): void;
}

interface ExampleBool extends ExampleMixed {
  public function __ampamp(ExampleBool $_): ExampleBool;
  public function __barbar(ExampleBool $_): ExampleBool;
  public function __bool(): bool;
  public function __exclamationMark(): ExampleBool;
}

interface ExampleString extends ExampleArraykey, XHPChild {
  public function __dot(ExampleString $_): ExampleString;
}

interface ExampleFloat extends ExampleMixed {}

final class ExampleContext {}

interface ExampleVoid {}

interface ExampleFunction<T> {
  public function __unwrap(): T;
}

interface ExampleShape<T> {
  public function __unwrap(): T;
}

abstract class ExampleKeyedCollection<+Tkey as ExampleArraykey, +Tvalue> {
  public static function __makeType<Tk as ExampleArraykey, Tv>(
    (Tk, Tv) ...$_
  ): ExampleKeyedCollection<Tk, Tv> {
    throw new Exception();
  }
}

abstract class ExampleKeyedCollectionMut<Tkey as ExampleArraykey, Tvalue> {
  public static function __makeType<Tk as ExampleArraykey, Tv>(
    (Tk, Tv) ...$_
  ): ExampleKeyedCollectionMut<Tk, Tv> {
    throw new Exception();
  }
}

// A second dsl for testing purposes
class ExampleDsl2 extends ExampleDsl {}

// The type of positions passed to the expression tree visitor.
type ExprPos = shape(...);

type ExampleDslExpression<T> = Spliceable<ExampleDsl, ExampleDsl::TAst, T>;

function concat_arg_list(vec<string> $args)[]: string {
  return implode(",", $args);
}

function concat_block(vec<string> $block)[]: string {
  return implode("\n", $block);
}
