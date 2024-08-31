<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public function bar(ExampleString $_, ExampleString $_two): ExampleInt {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = ExampleDsl`(MyClass $foo) ==> {
    $foo->bar("baz", "brr");
  }`;
}
