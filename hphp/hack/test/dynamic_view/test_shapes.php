<?hh // partial
function test() : int {
  $x = shape(NotRealClass::const => 5);


  hh_show($x);
  return $x;
}
