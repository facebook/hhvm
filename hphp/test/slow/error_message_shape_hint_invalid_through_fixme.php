<?hh
// fixme should not work
function foo(
  /* HH_FIXME[2015] */
  shape("foo" => int, "foo" => string) $x
): void {

}
