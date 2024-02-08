<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class Mock {
  const type TAst = mixed;

  public static function makeTree<TInfer>(
    ?ExprPos $pos,
    mixed $metadata,
    (function(Mock): Mock::TAst) $ast,
  ): MockExprTree<TInfer> {
    throw new Exception();
  }

  public static function symbolType<T>(T $_): HasUnwrap<T> {
    throw new Exception();
  }

  public function visitGlobalFunction<T>(?ExprPos $_, T $_): Mock::TAst {
    throw new Exception();
  }
  public function visitCall(
    ?ExprPos $_,
    Mock::TAst $_callee,
    vec<Mock::TAst> $_args,
  ): Mock::TAst {
    throw new Exception();
  }
}

final class MockExprTree<TInfer>
  implements Spliceable<Mock, Mock::TAst, TInfer> {
  public function __construct(
    ?ExprPos $pos,
    mixed $metadata,
    private (function(Mock): Mock::TAst) $ast,
  ) {}

  public function visit(Mock $v): Mock::TAst {
    return ($this->ast)($v);
  }

  public function toReturn(TInfer $_): void {
    throw new Exception();
  }
}

interface HasUnwrap<T> {
  public function __unwrap(): T;
}

function foo(): string {
  throw new Exception();
}

function test(): void {
  $y = Mock`foo()`;
  $y->toReturn("Hello");
}
