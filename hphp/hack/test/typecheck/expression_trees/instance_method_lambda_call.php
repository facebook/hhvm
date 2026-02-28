<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
interface IMyClass {
  public function bar(
  ): ExampleFunction<(function(ExampleString): ExampleVoid)>;
}

function foo(): void {
  $fun_call = ExampleDsl`(IMyClass $foo) ==> {
    $lambda = $foo->bar();
    $result = $lambda("baz");
  }`;
}
