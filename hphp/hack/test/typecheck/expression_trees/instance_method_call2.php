<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  public function bar(ExampleString $_): ExampleInt {
    throw new Exception();
  }
}

function getMyClass(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function(): MyClass)>> {
  throw new Exception();
}

function foo(): void {
  $fun_call = Code`() ==> {
    getMyClass()->bar("baz");
  }`;
}
