<?hh

/**
 * An example DSL for testing expression trees (ETs).
 *
 * Any class can be used an an expression tree visitor. It needs to implement
 * the methods shown here, and expression tree literals MyClass`...` will be
 * converted (at compile time) to calls on these methods.
 *
 * This .hhi file is only used when testing the type checker. Otherwise
 * every test file would need to include a class with all these methods.
 */
class ExampleDsl {
  const type TAst = mixed;

  // The desugared expression tree literal will call this method.
  public static function makeTree<TInfer>(
    ?ExprPos $pos,
    shape(
      'splices' => dict<string, mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>, ?'type' => (function (): TInfer),
    ) $metadata,
    (function(ExampleDsl): ExampleDsl::TAst) $ast,
  )[]: ExprTree<ExampleDsl, ExampleDsl::TAst, TInfer> {
    throw new Exception();
  }

  // The fooType() methods are used to typecheck the expression tree literals.
  // They do not require implementations.
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
    (function(ExampleContext): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, T>>) $_,
  ): T {
    throw new Exception();
  }
  public static function lambdaType<T>(
    T $_,
  ): ExampleFunction<T> {
    throw new Exception();
  }

  // The visitFoo methods are called at runtime when the expression tree literal
  // is evaluated. You will need to provide implementations of these methods,
  // but we've stubbed them for the sake of Hack tests.
  public function visitInt(?ExprPos $_, int $_): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitFloat(?ExprPos $_, float $_): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitBool(?ExprPos $_, bool $_): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitString(?ExprPos $_, string $_)[]: ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitNull(?ExprPos $_): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitBinop(
    ?ExprPos $_,
    ExampleDsl::TAst $lhs,
    string $op,
    ExampleDsl::TAst $rhs,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitUnop(
    ?ExprPos $_,
    ExampleDsl::TAst $operand,
    string $operator,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitLocal(?ExprPos $_, string $_): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitLambda(
    ?ExprPos $_,
    vec<string> $_args,
    vec<ExampleDsl::TAst> $_body,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitGlobalFunction<T>(
    ?ExprPos $_,
    (function(ExampleContext): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, T>>) $_,
  )[]: ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitStaticMethod<T>(
    ?ExprPos $_,
    (function(ExampleContext): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, T>>) $_,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitCall(
    ?ExprPos $_,
    ExampleDsl::TAst $_callee,
    vec<ExampleDsl::TAst> $_args,
  )[]: ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitAssign(
    ?ExprPos $_,
    ExampleDsl::TAst $_,
    ExampleDsl::TAst $_,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  public function visitTernary(
    ?ExprPos $_,
    ExampleDsl::TAst $_condition,
    ?ExampleDsl::TAst $_truthy,
    ExampleDsl::TAst $_falsy,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  // Statements.
  public function visitIf(
    ?ExprPos $_,
    ExampleDsl::TAst $_cond,
    vec<ExampleDsl::TAst> $_then_body,
    vec<ExampleDsl::TAst> $_else_body,
  ): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitWhile(
    ?ExprPos $_,
    ExampleDsl::TAst $_cond,
    vec<ExampleDsl::TAst> $_body,
  ): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitReturn(?ExprPos $_, ?ExampleDsl::TAst $_): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitFor(
    ?ExprPos $_,
    vec<ExampleDsl::TAst> $_,
    ?ExampleDsl::TAst $_,
    vec<ExampleDsl::TAst> $_,
    vec<ExampleDsl::TAst> $_,
  ): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitBreak(?ExprPos $_): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitContinue(?ExprPos $_): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitPropertyAccess(
    ?ExprPos $_,
    ExampleDsl::TAst $_,
    string $_,
  ): ExampleDsl::TAst {
    throw new Exception();
  }
  public function visitXhp(
    ?ExprPos $_,
    string $_,
    dict<string, ExampleDsl::TAst> $_,
    vec<ExampleDsl::TAst> $_,
  ): ExampleDsl::TAst {
    throw new Exception();
  }

  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    ExampleDslExpression<T> $_,
  )[]: ExampleDsl::TAst {
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

// The type of positions passed to the expression tree visitor.
type ExprPos = shape(...);

// Type declarations used when checking DSL expressions.
interface ExampleMixed {
  public function __tripleEquals(ExampleMixed $_): ExampleBool;
  public function __notTripleEquals(ExampleMixed $_): ExampleBool;
}
interface ExampleInt extends ExampleMixed {
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

interface ExampleString extends ExampleMixed, XHPChild {
  public function __dot(ExampleString $_): ExampleString;
}

interface ExampleFloat extends ExampleMixed {}

final class ExampleContext {}

interface ExampleVoid {}

interface ExampleFunction<T> {
  public function __unwrap(): T;
}

type ExampleDslExpression<T> = Spliceable<ExampleDsl, ExampleDsl::TAst, T>;
