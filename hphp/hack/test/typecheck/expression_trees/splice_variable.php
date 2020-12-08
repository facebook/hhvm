<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExprTree<Code, Code::TAst, T> {
  throw new Exception();
}

function test(): void {
  $x = 1;

  // Expression Trees do not inherit local variables from the outer scope
  // But splices do
  $_ = Code`__splice__(lift($x))`;
}

//// BEGIN DEFS
// Placeholder definition so we don't get naming/typing errors.
class Code {
  const type TAst = mixed;
  // Lifting literals.
  public static function intLiteral(
    int $_,
  ): ExprTree<Code, Code::TAst, ExampleInt> {
    throw new Exception();
  }
  public static function floatLiteral(
    float $_,
  ): ExprTree<Code, Code::TAst, ExampleFloat> {
    throw new Exception();
  }
  public static function boolLiteral(bool $_):
    ExprTree<Code, Code::TAst, ExampleBool>
  {
    throw new Exception();
  }
  public static function stringLiteral(string $_):
    ExprTree<Code, Code::TAst, ExampleString>
  {
    throw new Exception();
  }
  public static function nullLiteral(): ExprTree<Code, Code::TAst, null> {
    throw new Exception();
  }

  // Symbols
  public static function symbol<T>(
    string $_,
    (function(ExampleContext): ExprTree<Code, Code::TAst, T>) $_,
  ): ExprTree<Code, Code::TAst, T> {
    throw new Exception();
  }

  // Expressions
  public function localVar(?ExprPos $_, string $_): Code::TAst {
    throw new Exception();
  }
  public function lambdaLiteral(
    ?ExprPos $_,
    vec<string> $_args,
    vec<Code::TAst> $_body,
  ): Code::TAst {
    throw new Exception();
  }

  // Operators
  public function methCall(
    ?ExprPos $_,
    Code::TAst $_,
    string $_,
    vec<Code::TAst> $_,
  ): Code::TAst {
    throw new Exception();
  }

  // Old style operators
  public function call<T>(
    ?ExprPos $_,
    Code::TAst $_callee,
    vec<Code::TAst> $_args,
  ): Code::TAst {
    throw new Exception();
  }

  public function assign(
    ?ExprPos $_,
    Code::TAst $_,
    Code::TAst $_,
  ): Code::TAst {
    throw new Exception();
  }

  public function ternary(
    ?ExprPos $_,
    Code::TAst $_condition,
    ?Code::TAst $_truthy,
    Code::TAst $_falsy,
  ): Code::TAst {
    throw new Exception();
  }

  // Statements.
  public function ifStatement(
    ?ExprPos $_,
    Code::TAst $_cond,
    vec<Code::TAst> $_then_body,
    vec<Code::TAst> $_else_body,
  ): Code::TAst {
    throw new Exception();
  }
  public function whileStatement(
    ?ExprPos $_,
    Code::TAst $_cond,
    vec<Code::TAst> $_body,
  ): Code::TAst {
    throw new Exception();
  }
  public function returnStatement(
    ?ExprPos $_,
    ?Code::TAst $_,
  ): Code::TAst {
    throw new Exception();
  }
  public function forStatement(
    ?ExprPos $_,
    vec<Code::TAst> $_,
    Code::TAst $_,
    vec<Code::TAst> $_,
    vec<Code::TAst> $_,
  ): Code::TAst {
    throw new Exception();
  }
  public function breakStatement(?ExprPos $_): Code::TAst {
    throw new Exception();
  }
  public function continueStatement(?ExprPos $_,): Code::TAst {
    throw new Exception();
  }

  // Splice
  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    ExprTree<Code, Code::TAst, T> $_,
  ): Code::TAst {
    throw new Exception();
  }

  // TODO: Discard unsupported syntax nodes while lowering
  public function unsupportedSyntax(string $msg): Code::TAst {
    throw new Exception($msg);
  }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private ?ExprPos $pos,
    private string $filepath,
    private dict<string, mixed> $spliced_values,
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
