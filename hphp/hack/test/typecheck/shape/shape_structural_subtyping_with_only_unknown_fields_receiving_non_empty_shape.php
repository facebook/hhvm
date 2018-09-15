<?hh // strict

// Any shape should satisfy this interface.
type ShapeWithUnknownFields = shape(...);

function foo(ShapeWithUnknownFields $argument): void {}

function bar(): void {
  foo(shape('a' => 12));
}
