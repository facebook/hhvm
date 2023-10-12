<?hh // strict

trait TFoo {}

function f(mixed $x): void {
  $x as TFoo;
}
