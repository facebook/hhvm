<?hh

function foo(
    /* HH_FIXME[4032] */
    $x,
    /* HH_FIXME[4032] */
    $y,
): void {
  bar($x, $y);
}

function bar(int $x, string $y): void {}
