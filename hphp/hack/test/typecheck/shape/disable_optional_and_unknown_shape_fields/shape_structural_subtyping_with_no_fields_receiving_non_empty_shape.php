<?hh // strict

// Any shape should satisfy this interface.
type ShapeWithNoFields = shape();

function foo(ShapeWithNoFields $argument): void {}

function bar(): void {
  foo(shape('a' => 12));
}
