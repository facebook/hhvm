<?hh

class Code {
  const type TAst = mixed;

  public static function makeTree<<<__Explicit>> TInfer>(
    ?ExprPos $pos,
    shape(
      'splices' => dict<string, mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>,
    ) $metadata,
    (function(Code): Code::TAst) $ast,
  ): ExprTree<Code, Code::TAst, TInfer> {
    throw new Exception();
  }

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

  // Desugared nodes (These should be implemented)
  public function visitInt(?ExprPos $_, int $_): Code::TAst {
    throw new Exception();
  }
  public function visitFloat(?ExprPos $_, float $_): Code::TAst {
    throw new Exception();
  }
  public function visitBool(?ExprPos $_, bool $_): Code::TAst {
    throw new Exception();
  }
  public function visitString(?ExprPos $_, string $_): Code::TAst {
    throw new Exception();
  }
  public function visitNull(?ExprPos $_): Code::TAst {
    throw new Exception();
  }

  public function visitBinop(
    ?ExprPos $_,
    Code::TAst $lhs,
    string $op,
    Code::TAst $rhs,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitUnop(
    ?ExprPos $_,
    Code::TAst $operand,
    string $operator,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitLocal(?ExprPos $_, string $_): Code::TAst {
    throw new Exception();
  }

  public function visitLambda(
    ?ExprPos $_,
    vec<string> $_args,
    vec<Code::TAst> $_body,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitGlobalFunction<T>(
    ?ExprPos $_,
    (function(ExampleContext): Awaitable<ExprTree<Code, Code::TAst, T>>) $_,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitStaticMethod<T>(
    ?ExprPos $_,
    (function(ExampleContext): Awaitable<ExprTree<Code, Code::TAst, T>>) $_,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitCall(
    ?ExprPos $_,
    Code::TAst $_callee,
    vec<Code::TAst> $_args,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitAssign(
    ?ExprPos $_,
    Code::TAst $_,
    Code::TAst $_,
  ): Code::TAst {
    throw new Exception();
  }

  public function visitTernary(
    ?ExprPos $_,
    Code::TAst $_condition,
    ?Code::TAst $_truthy,
    Code::TAst $_falsy,
  ): Code::TAst {
    throw new Exception();
  }

  // Statements.
  public function visitIf(
    ?ExprPos $_,
    Code::TAst $_cond,
    vec<Code::TAst> $_then_body,
    vec<Code::TAst> $_else_body,
  ): Code::TAst {
    throw new Exception();
  }
  public function visitWhile(
    ?ExprPos $_,
    Code::TAst $_cond,
    vec<Code::TAst> $_body,
  ): Code::TAst {
    throw new Exception();
  }
  public function visitReturn(
    ?ExprPos $_,
    ?Code::TAst $_,
  ): Code::TAst {
    throw new Exception();
  }
  public function visitFor(
    ?ExprPos $_,
    vec<Code::TAst> $_,
    ?Code::TAst $_,
    vec<Code::TAst> $_,
    vec<Code::TAst> $_,
  ): Code::TAst {
    throw new Exception();
  }
  public function visitBreak(?ExprPos $_): Code::TAst {
    throw new Exception();
  }
  public function visitContinue(?ExprPos $_,): Code::TAst {
    throw new Exception();
  }

  // Splice
  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    Spliceable<Code, Code::TAst, T> $_,
  ): Code::TAst {
    throw new Exception();
  }
}

interface Spliceable<TVisitor, TResult, +TInfer> {
  public function visit(TVisitor $v): TResult;
}

final class ExprTree<TVisitor, TResult, +TInfer>
  implements Spliceable<TVisitor, TResult, TInfer> {
  public function __construct(
    private ?ExprPos $pos,
    private shape(
      'splices' => dict<string, mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>,
    ) $metadata,
    private (function(TVisitor): TResult) $ast,
    private (function(): TInfer) $err,
  ) {}

  public function visit(TVisitor $v): TResult {
    return ($this->ast)($v);
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

final class ExampleContext {}

abstract class ExampleVoid {}
