<?hh // partial
function foo() {
  return 5;
}
function test() : array<int> {
  $x = foo();
  hh_show($x);
  $y = array();
  $y[] = $x;
  hh_show($y);
  return $y;
}
