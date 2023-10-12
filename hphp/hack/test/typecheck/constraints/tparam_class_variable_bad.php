<?hh // strict

function test<T>(classname<T> $c): void {
  $_ = $c::$doesNotExist;
}
