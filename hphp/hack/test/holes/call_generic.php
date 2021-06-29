<?hh

function generic_bounded<T as num>(T $x, T $y): void {}

function call_generic(int $x, string $y): void {
  /* HH_FIXME[4323] */
  generic_bounded($x, $y);
}

function call_generic_cast(int $x, string $y): void {
  /* HH_FIXME[4417] */
  generic_bounded($x, \HH\FIXME\UNSAFE_CAST<string,float>($y));
}
