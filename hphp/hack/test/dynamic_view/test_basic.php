<?hh // partial
function foo()  {
  $x = " hello ";
  return 5;
}


function test() : int {
  // partial class
  $x = foo();
  hh_show($x);
  return $x;
}
