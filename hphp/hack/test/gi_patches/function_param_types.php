<?hh

/* HH_FIXME[4032] */
function foo($x, $y): void {
  bar($x, $y);
}

function bar(int $x, string $y): void {}
