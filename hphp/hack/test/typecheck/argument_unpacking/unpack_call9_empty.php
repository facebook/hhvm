<?hh

function test(): void {
  $args = array("hello");
  // HHVM doesn't support unpacking into empty
  empty(...$args);
}
