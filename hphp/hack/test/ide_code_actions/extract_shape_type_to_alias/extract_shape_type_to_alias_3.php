<?hh

type TShape = shape('a' => int);

class A {
  // we don't provide the refactoring unless a shape hint literal is selected
  public function foo(/*range-start*/TShape/*range-end*/ $_): void {}
}
