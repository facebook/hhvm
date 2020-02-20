<?hh // partial

function test(): void {
  $args = varray["hello"];
  // HHVM doesn't support unpacking into isset
  isset(...$args);
}
