<?hh // partial

function lol(mixed $x): string {
  return 'lol';
}

function f(): int {
  $n = null;
  /* HH_FIXME[4110] Allow two FIXMEs one after another */
  /* HH_FIXME[4064] Allow two FIXMEs one after another */
  return lol($n->a);
}
