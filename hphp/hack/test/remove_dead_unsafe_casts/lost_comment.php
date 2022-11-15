<?hh

function f(int $i): void {
  /* We should have a comment on the next line. */
  HH\FIXME\UNSAFE_CAST<mixed, int>(
    /* We would like to retain this comment. It might be a FIXME! */
    $i
  );
}
