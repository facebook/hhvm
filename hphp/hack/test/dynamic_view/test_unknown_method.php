<?hh // partial

function test() : (int, int) {
  // something isn't defined
  $x = something();
  /* HH_IGNORE_ERROR[2049] */ hh_show($x);
  return $x;
}
