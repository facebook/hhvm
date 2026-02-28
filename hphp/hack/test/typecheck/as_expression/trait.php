<?hh

trait TFoo {}

function f(mixed $x): void {
  $x as TFoo;
}
