<?hh

function test<T>(classname<T> $c): void {
  $c::doesNotExist();
}
