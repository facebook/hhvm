<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static function bar(): int { return 1; }
}

const int MY_CONST = 1;

function foo(): void {
  // Ban binary operators.
  $x = Code`1 << 2`;

  // Ban static methods and instantiation.
  $y = Code`Foo::bar()`;
  $z = Code`new Foo()`;

  // Ban globals.
  $g = Code`MY_CONST + 1`;

  // Ban PHP-style lambdas.
  $f = Code`function() { return 1; }`;

  // Ban do-while and foreach loops.
  $f = Code`() ==> { do {} while(true); }`;
  $f = Code`(vec<int> $items) ==> { foreach ($items as $_) {} }`;

  // Ban lambdas with default arguments.
  $f = Code`(int $x = 1) ==> { return $x; }`;

  // Ban assignment to things that aren't simple variables.
  $f = Code`(dynamic $x) ==> { $x[0] = 1; }`;
  $f = Code`(dynamic $x) ==> { $x->foo = 1; }`;

  // Ban assignments that mutate a local.
  $f = Code`(int $x) ==> { $x += 1; }`;
}

//// BEGIN DEFS
// Placeholder definition so we don't get naming/typing errors.
final class Code {
  const type TAst = mixed;
  // Simple literals.
  public function intLiteral(?ExprPos $_, int $_): this::TAst {
    throw new Exception();
  }
  public function boolLiteral(?ExprPos $_, bool $_): this::TAst {
    throw new Exception();
  }
  public function stringLiteral(?ExprPos $_, string $_): this::TAst {
    throw new Exception();
  }
  public function nullLiteral(?ExprPos $_): this::TAst {
    throw new Exception();
  }
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
  public function plus(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function ampamp(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function barbar(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function exclamationMark(
    ?ExprPos $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function call(
    ?ExprPos $_,
    string $_fnName,
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
//// END DEFS
