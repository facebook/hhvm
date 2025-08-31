<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

namespace Bar {

  class MyExampleDsl {
    const type TAst = mixed;

    public static function makeTree<TInfer>(
      ?\ExprPos $pos,
      shape(
        'splices' => dict<string, mixed>,
        'functions' => vec<mixed>,
        'static_methods' => vec<mixed>,
        ?'type' => TInfer,
        'variables' => vec<string>,
        'lexically_enclosing_tree' => ?\ExprPos,
      ) $metadata,
      (function(MyExampleDsl): MyExampleDsl::TAst) $ast,
    ): \ExampleExpression<TInfer> {
      throw new \Exception();
    }

    // Virtual types (These do not have to be implemented)
    public static function intType(): \ExampleInt {
      throw new \Exception();
    }

    // Desugared nodes (These should be implemented)
    public function visitInt(?\ExprPos $_, int $_): MyExampleDsl::TAst {
      throw new \Exception();
    }
  }

} // namespace Bar

namespace {

  function test(): void {
    Bar\MyExampleDsl`1`;
  }

} // namespace
