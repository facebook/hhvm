<?hh // partial
function test() : int {
  $x = shape(NotRealClass::const => 5);


  /* HH_IGNORE_ERROR[2049] */ hh_show($x);
  return $x;
}
