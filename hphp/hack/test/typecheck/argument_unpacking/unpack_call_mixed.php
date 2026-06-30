<?hh

function call(mixed $f, ?vec<mixed> $args): void {
  /* HH_FIXME[4009] We want to test the unpacking error */
  $f(...$args);
}
