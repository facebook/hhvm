<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definition so we don't get naming/typing errors.
class Code {
  const type TAst = mixed;

  public function boolLiteral(bool $_): this::TAst {
    throw new Exception();
  }

  public function whileStatement(
    this::TAst $_cond,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }

  public function lambdaLiteral(
    vec<string> $_args,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }

  public function continueStatement(): this::TAst {
    throw new Exception();
  }

}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private (function(TVisitor): TResult) $x,
    private (function(): TInfer) $err,
  ) {}
}

function foo(): void {
  $n = Code`() ==> {
    while(true) {
      continue;
    }
  }`;
}
