<?hh

function test<T>(class<T> $c): void {
  $_ = $c::DOES_NOT_EXIST;
}
