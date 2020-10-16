<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definitions so we don't get naming/typing errors.
class Code {
  const type TAst = mixed;
  // Simple literals.
  public function intLiteral(int $_): this::TAst {
    throw new Exception();
  }
  public function boolLiteral(bool $_): this::TAst {
    throw new Exception();
  }
  public function stringLiteral(string $_): this::TAst {
    throw new Exception();
  }
  public function localVar(string $_): this::TAst {
    throw new Exception();
  }

  // Operators
  public function plus(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function call(string $_fnName, vec<this::TAst> $_args): this::TAst {
    throw new Exception();
  }

  // Statements.
  public function assign(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function ifStatement(
    this::TAst $_cond,
    vec<this::TAst> $_then_body,
    vec<this::TAst> $_else_body,
  ): this::TAst {
    throw new Exception();
  }
  public function whileStatement(
    this::TAst $_cond,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }
  public function returnStatement(?this::TAst $_): this::TAst {
    throw new Exception();
  }

  public function lambdaLiteral(
    vec<string> $_args,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }

  // TODO: it would be better to discard unsupported syntax nodes during lowering.
  public function unsupportedSyntax(string $msg): this::TAst {
    throw new Exception($msg);
  }
}
class Foo {
  public static function bar(): int { return 1; }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private (function(TVisitor): TResult) $x,
    private (function(): TInfer) $err,
  ) {}
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

  // Ban loops other than while.
  $f = Code`() ==> { do {} while(true); }`;
  $f = Code`() ==> { for($i = 0; $i < 10; $i++) {} }`;

  // Ban lambdas with default arguments.
  $f = Code`(int $x = 1) ==> { return $x; }`;

  // Ban assignment to things that aren't simple variables.
  $f = Code`(dynamic $x) ==> { $x[0] = 1; }`;
  $f = Code`(dynamic $x) ==> { $x->foo = 1; }`;

  // Ban assignments that mutate a local.
  $f = Code`(int $x) ==> { $x += 1; }`;
}
