<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public function bar(ExampleString $_): ExampleInt {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = Code`(MyClass $foo) ==> {
    $foo->bar("baz");
  }`;
}
