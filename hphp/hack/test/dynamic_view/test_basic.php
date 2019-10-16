<?hh // partial
function foo()  {
  $x = " hello ";
  return 5;
}


function test() : (int, int) {
  // partial class
  $x = foo();
  /* HH_IGNORE_ERROR[2049] */ hh_show($x);
  return $x;
}
