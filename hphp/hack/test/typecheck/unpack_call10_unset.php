<?hh

function test(): void {
  $args = array("hello");
  // HHVM doesn't support unpacking into unset
  unset(...$args);
}
