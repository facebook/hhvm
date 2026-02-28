<?hh

function test<T>(class<T> $c): void {
  $_ = $c::$doesNotExist;
}
