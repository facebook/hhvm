<?hh // strict

class C {}

function test<T as C>(classname<T> $c): void {
  $c::doesNotExist();
}
