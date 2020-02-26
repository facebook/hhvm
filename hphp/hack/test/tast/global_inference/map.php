<?hh // partial

/* HH_FIXME[4101] */
function foo(Map $m) {
  /* HH_FIXME[4008] */
  $m["hey"] = 40;
  /* HH_FIXME[4008] */
  return $m["ho"];
}
