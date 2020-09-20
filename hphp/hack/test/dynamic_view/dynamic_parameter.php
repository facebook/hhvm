<?hh // partial
function takes_int((int, int) $x) : void {}
function foo($x) {
  /* HH_IGNORE_ERROR[2049] */ hh_show($x);
  takes_int($x);
  /* HH_IGNORE_ERROR[2049] */ hh_show($x);
}
