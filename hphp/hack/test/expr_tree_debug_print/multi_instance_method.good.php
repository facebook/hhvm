<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public function bar(ExampleString $_, ExampleString $_two): MyClass {
    throw new Exception();
  }

  public function waldo(ExampleString $_, ExampleString $_two): MyClass {
    throw new Exception();
  }
}

function foo(): void {
  $fun_call = ExampleDsl`(MyClass $foo) ==> {
    $foo->bar("baz", "brr")->waldo("baz", "brr");
  }`;
}
