<?hh

function test(string $key): void {
  $s = shape();
  /* HH_FIXME[4051] - there should only be one error,
   and single HH_FIXME should squash it */
  $s[$key] = 4;
}
