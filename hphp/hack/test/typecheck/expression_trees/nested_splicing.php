<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class Code {
  const type TAst = mixed;

  public function splice(mixed $_): this::TAst {
    throw new Exception();
  }
}

final class ExprTree<TVisitor, TResult>{
  public function __construct(
    private (function(TVisitor): TResult) $x,
  ) {}
}

function test(): void {
  Code`__splice__(__splice__(4))`;
}
