<?hh

<<file: __EnableUnstableFeatures('expression_trees', 'expression_tree_nullsafe_obj_get')>>

function test_error(): void {
  ExampleDsl`(?MyState $x) ==> {
    // Expected error: $x is nullable, $x?->bar(1) is also nullable
    return 1 + $x?->bar(1);
  }`;
}

abstract class MyState {
  public function foo(ExampleInt $x): void {}
  public function bar(ExampleInt $x): ExampleInt {
    return $x;
  }
}
