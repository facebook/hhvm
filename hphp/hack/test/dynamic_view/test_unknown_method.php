<?hh // partial

function test() : int {
  // something isn't defined
  $x = something();
  hh_show($x);
  return $x;
}
