<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

namespace Bar {

class MyCode {
  const type TAst = mixed;

  public static function makeTree<<<__Explicit>> TInfer>(
    ?\ExprPos $pos,
    shape(
      'splices' => dict<string, mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>,
    ) $metadata,
    (function(MyCode): MyCode::TAst) $ast,
  ): \ExprTree<MyCode, MyCode::TAst, TInfer> {
    throw new \Exception();
  }

  // Virtual types (These do not have to be implemented)
  public static function intType(): \ExampleInt {
    throw new \Exception();
  }

  // Desugared nodes (These should be implemented)
  public function visitInt(?\ExprPos $_, int $_): MyCode::TAst {
    throw new \Exception();
  }
}

} // namespace Bar

namespace {

function test(): void {
    Bar\MyCode`1`;
}

} // namespace
