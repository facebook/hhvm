<?hh

function with_inout(inout int $i): void {}

function call_inout(float $f): void {
  /* HH_FIXME[4110] */
  with_inout(inout $f);
}
