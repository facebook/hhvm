<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class Code {
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

  public function splice(mixed $_): this::TAst {
    throw new Exception();
  }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private (function(TVisitor): TResult) $x,
    private (function(): TInfer) $err,
  ) {}
}

function test(): void {
  $x = Code`4`;
  $y = Code`5`;
  Code`__splice__($x) + __splice__($y)`;
}
