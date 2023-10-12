<?hh // strict

function test<T>(classname<T> $c): void {
  $_ = $c::DOES_NOT_EXIST;
}
