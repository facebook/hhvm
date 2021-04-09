<?hh

function foo(int $x): void {}

function call_dead_code(): void {
  $x = 1;
  if (true) {
    $x = 'string';
    /* HH_FIXME[4110] */
    foo($x);
  } else {
    $x = 'string';
    foo($x);
  }
}

function call_dead_code_cast(): void {
  $x = 1;
  if (true) {
    $x = 'string';
    /* HH_FIXME[4417] */
    foo(unsafe_cast<string,int>($x));
  } else {
    $x = 'string';
    foo($x);
  }
}
