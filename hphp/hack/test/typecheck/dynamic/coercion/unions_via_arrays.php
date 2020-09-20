<?hh // strict

function test(dynamic $x): void {
  $v = vec[$x, "foo"];
  $y = $v[0];
  $y->someMethod(); // should fail, since $y could be string
}
