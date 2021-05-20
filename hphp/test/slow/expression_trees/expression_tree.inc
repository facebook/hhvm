<?hh

type ExprTreeInfo = shape(
  'splices' => dict<string, mixed>,
  'functions' => vec<mixed>,
  'static_methods' => vec<mixed>,
);

function concat_arg_list(vec<string> $args): string {
  return implode(",", $args);
}

function concat_block(vec<string> $block): string {
  return implode("\n", $block);
}

class Code {
  const type TAst = string;

    // Virtual types (These do not have to be implemented)
  public static function intType(): ExampleInt {
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
    (function(ExampleContext): Awaitable<ExprTree<Code, Code::TAst, T>>) $_,
  ): T {
    throw new Exception();
  }

  public static function makeTree<TVisitor as Code, TInfer>(
    ?ExprPos $pos,
    ExprTreeInfo $metadata,
    (function(TVisitor): Code::TAst) $ast,
  ): ExprTree<TVisitor, Code::TAst, TInfer> {
    return new ExprTree($pos, $metadata, $ast);
  }

  // Desugared nodes (These should be implemented)
  public function visitInt(?ExprPos $_, int $i): string {
    return (string)$i;
  }
  public function visitFloat(?ExprPos $_, float $f): string {
    return (string)$f;
  }
  public function visitBool(?ExprPos $_, bool $b): string {
    return $b ? "true" : "false";
  }
  public function visitString(?ExprPos $_, string $s): string {
    return "\"$s\"";
  }
  public function visitNull(?ExprPos $_): string {
    return "null";
  }

  public function visitGlobalFunction<T>(
    ?ExprPos $_,
    (function(ExampleContext): Awaitable<ExprTree<Code, Code::TAst, T>>) $_,
  ): Code::TAst {
    return "function_ptr";
  }

  public function visitStaticMethod<T>(
    ?ExprPos $_,
    (function(ExampleContext): Awaitable<ExprTree<Code, Code::TAst, T>>) $_,
  ): Code::TAst {
    return "function_ptr";
  }

  // Expressions
  public function visitLocal(?ExprPos $_, string $local): Code::TAst {
    return $local;
  }

  public function visitLambda(
    ?ExprPos $_,
    vec<string> $args,
    vec<Code::TAst> $body,
  ): Code::TAst {
    return "(".concat_arg_list($args).")"." ==> {\n".concat_block($body)."}";
  }

  // New operators
  public function visitBinop(
    ?ExprPos $_,
    string $lhs,
    string $op,
    string $rhs,
  ): string {
    return "$lhs $op $rhs";
  }

  public function visitUnop(
    ?ExprPos $_,
    string $op,
    string $operand,
  ): string {
    return "$op $operand";
  }

  public function visitCall<T>(
    ?ExprPos $_,
    Code::TAst $callee,
    vec<Code::TAst> $args,
  ): Code::TAst {
    return $callee."(".concat_arg_list($args).")";
  }

  public function visitAssign(
    ?ExprPos $_,
    Code::TAst $local,
    Code::TAst $value,
  ): Code::TAst {
    return $local." = ".$value.";";
  }

  public function visitTernary(
    ?ExprPos $_,
    Code::TAst $condition,
    ?Code::TAst $truthy,
    Code::TAst $falsy,
  ): Code::TAst {
    return $condition." ? ".$truthy." : ".$falsy;
  }

  public function visitIf(
    ?ExprPos $_,
    Code::TAst $cond,
    vec<Code::TAst> $then_body,
    vec<Code::TAst> $else_body,
  ): Code::TAst {
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
    Code::TAst $cond,
    vec<Code::TAst> $body,
  ): Code::TAst {
    return "while (".$cond.") {\n".concat_block($body)."}";
  }

  public function visitReturn(?ExprPos $_, ?Code::TAst $return_val): Code::TAst {
    if ($return_val is null) {
      return "return;";
    }
    return "return ".$return_val.";";
  }

  public function visitFor(
    ?ExprPos $_,
    vec<Code::TAst> $initializers,
    ?Code::TAst $cond,
    vec<Code::TAst> $increments,
    vec<Code::TAst> $body,
  ): Code::TAst {
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

  public function visitBreak(?ExprPos $_): Code::TAst {
    return "break;";
  }

  public function visitContinue(?ExprPos $_): Code::TAst {
    return "continue;";
  }

  // Splice
  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    Spliceable<Code, Code::TAst, T> $splice_val,
  ): Code::TAst {
    return "\${".($splice_val->visit($this))."}";
  }
}

interface Spliceable<TVisitor, TResult, +TInfer> {
  public function visit(TVisitor $v): TResult;
}

final class ExprTree<TVisitor, TResult, +TInfer>
  implements Spliceable<TVisitor, TResult, TInfer> {
  public function __construct(
    private ?ExprPos $pos,
    private ExprTreeInfo $metadata,
    private (function(TVisitor): TResult) $ast,
  ) {}

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

type ExprPos = shape(...);

abstract class ExampleMixed {
  public abstract function __tripleEquals(ExampleMixed $_): ExampleBool;
  public abstract function __notTripleEquals(ExampleMixed $_): ExampleBool;
}
abstract class ExampleInt extends ExampleMixed {
  public abstract function __plus(ExampleInt $_): ExampleInt;
  public abstract function __minus(ExampleInt $_): ExampleInt;
  public abstract function __star(ExampleInt $_): ExampleInt;
  public abstract function __slash(ExampleInt $_): ExampleInt;
  public abstract function __percent(ExampleInt $_): ExampleInt;
  public abstract function __negate(): ExampleInt;

  public abstract function __lessThan(ExampleInt $_): ExampleBool;
  public abstract function __lessThanEqual(ExampleInt $_): ExampleBool;
  public abstract function __greaterThan(ExampleInt $_): ExampleBool;
  public abstract function __greaterThanEqual(ExampleInt $_): ExampleBool;

  public abstract function __amp(ExampleInt $_): ExampleInt;
  public abstract function __bar(ExampleInt $_): ExampleInt;
  public abstract function __caret(ExampleInt $_): ExampleInt;
  public abstract function __lessThanLessThan(ExampleInt $_): ExampleInt;
  public abstract function __greaterThanGreaterThan(ExampleInt $_): ExampleInt;
  public abstract function __tilde(): ExampleInt;
}

abstract class ExampleBool extends ExampleMixed {
  public abstract function __ampamp(ExampleBool $_): ExampleBool;
  public abstract function __barbar(ExampleBool $_): ExampleBool;
  public abstract function __bool(): bool;
  public abstract function __exclamationMark(): ExampleBool;
}

abstract class ExampleString extends ExampleMixed {
  public abstract function __dot(ExampleString $_): ExampleString;
}

abstract class ExampleFloat extends ExampleMixed {}

abstract class ExampleVoid extends ExampleMixed {}

final class ExampleContext {}

function print_et<TInfer>(ExprTree<Code, Code::TAst, TInfer> $et): void {
  $visitor = new Code();
  $string = $et->visit($visitor);
  echo($string."\n");
}
