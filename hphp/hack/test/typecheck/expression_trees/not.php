<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definition so we don't get naming/typing errors.
class Code {
  const type TAst = mixed;
  // Simple literals.
  public function boolLiteral(bool $_): this::TAst {
    throw new Exception();
  }

  // Operators
  public function exclamationMark(this::TAst $_): this::TAst {
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
  Code`!false`;
}
