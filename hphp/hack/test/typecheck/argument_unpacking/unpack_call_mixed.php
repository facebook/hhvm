<?hh

function call(mixed $f, ?vec<mixed> $args): void {
  /* HH_IGNORE_ERROR[4009] We want to test the unpacking error */
  $f(...$args);
}
