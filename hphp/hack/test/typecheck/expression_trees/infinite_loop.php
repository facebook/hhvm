<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $infinite_for =
    Code`() ==> {
      for (;;) {}
    }`;
}

//// BEGIN DEFS
// Placeholder definition so we don't get naming/typing errors.
final class Code {
  const type TAst = mixed;
  // Lifting literals.
  public static function intLiteral(
    int $_,
  ): ExprTree<this, this::TAst, ExampleInt> {
    throw new Exception();
  }
  public static function floatLiteral(
    float $_,
  ): ExprTree<this, this::TAst, ExampleFloat> {
    throw new Exception();
  }
  public static function boolLiteral(bool $_):
    ExprTree<this, this::TAst, ExampleBool>
  {
    throw new Exception();
  }
  public static function stringLiteral(string $_):
    ExprTree<this, this::TAst, ExampleString>
  {
    throw new Exception();
  }
  public static function nullLiteral(): ExprTree<this, this::TAst, null> {
    throw new Exception();
  }

  // Symbols
  public static function symbol<T>(
    string $_,
    (function(ExampleContext): ExprTree<this, this::TAst, T>) $_,
  ): ExprTree<this, this::TAst, T> {
    throw new Exception();
  }

  // Expressions
  public function localVar(?ExprPos $_, string $_): this::TAst {
    throw new Exception();
  }
  public function lambdaLiteral(
    ?ExprPos $_,
    vec<string> $_args,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }

  // Operators
  public function methCall(
    ?ExprPos $_,
    this::TAst $_,
    string $_,
    vec<this::TAst> $_,
  ): this::TAst {
    throw new Exception();
  }

  // Old style operators
  public function call<T>(
    ?ExprPos $_,
    this::TAst $_callee,
    vec<this::TAst> $_args,
  ): this::TAst {
    throw new Exception();
  }

  public function assign(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }

  public function ternary(
    ?ExprPos $_,
    this::TAst $_condition,
    ?this::TAst $_truthy,
    this::TAst $_falsy,
  ): this::TAst {
    throw new Exception();
  }

  // Statements.
  public function ifStatement(
    ?ExprPos $_,
    this::TAst $_cond,
    vec<this::TAst> $_then_body,
    vec<this::TAst> $_else_body,
  ): this::TAst {
    throw new Exception();
  }
  public function whileStatement(
    ?ExprPos $_,
    this::TAst $_cond,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }
  public function returnStatement(
    ?ExprPos $_,
    ?this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function forStatement(
    ?ExprPos $_,
    vec<this::TAst> $_,
    this::TAst $_,
    vec<this::TAst> $_,
    vec<this::TAst> $_,
  ): this::TAst {
    throw new Exception();
  }
  public function breakStatement(?ExprPos $_): this::TAst {
    throw new Exception();
  }
  public function continueStatement(?ExprPos $_,): this::TAst {
    throw new Exception();
  }

  // Splice
  public function splice<T>(
    ?ExprPos $_,
    ExprTree<this, this::TAst, T> $_,
  ): this::TAst {
    throw new Exception();
  }

  // TODO: Discard unsupported syntax nodes while lowering
  public function unsupportedSyntax(string $msg): this::TAst {
    throw new Exception($msg);
  }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private ?ExprPos $pos,
    private string $filepath,
    private (function(TVisitor): TResult) $x,
    private (function(): TInfer) $err,
  ) {}
}

final class ExprPos {
  public function __construct(
    private int $begin_line,
    private int $begin_col,
    private int $end_line,
    private int $end_col,
  ) {}
}

final class ExampleInt {
  public function __plus(ExampleInt $_): ExampleInt {
    throw new Exception();
  }
  public function __minus(ExampleInt $_): ExampleInt {
    throw new Exception();
  }
  public function __star(ExampleInt $_): ExampleInt {
    throw new Exception();
  }
  public function __slash(ExampleInt $_): ExampleInt {
    throw new Exception();
  }

  public function __lessThan(ExampleInt $_): ExampleBool {
    throw new Exception();
  }

  public function __lessThanEqual(ExampleInt $_): ExampleBool {
    throw new Exception();
  }

  public function __greaterThan(ExampleInt $_): ExampleBool {
    throw new Exception();
  }

  public function __greaterThanEqual(ExampleInt $_): ExampleBool {
    throw new Exception();
  }

  public function __tripleEquals(ExampleInt $_): ExampleBool {
    throw new Exception();
  }

  public function __notTripleEquals(ExampleInt $_): ExampleBool {
    throw new Exception();
  }
}

final class ExampleBool {
  public function __ampamp(ExampleBool $_): ExampleBool {
    throw new Exception();
  }

  public function __barbar(ExampleBool $_): ExampleBool {
    throw new Exception();
  }

  public function __bool(): bool {
    throw new Exception();
  }

  public function __exclamationMark(): ExampleBool {
    throw new Exception();
  }
}

final class ExampleString {}

final class ExampleContext {}

final class ExampleFloat {}

//// END DEFS
