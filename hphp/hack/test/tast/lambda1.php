<?hh
function test_lambda1(): void {
  $s = 'foo';
  $f = $n ==> { return $n . $s . '\n'; };
  $x = $f(4);
  $y = $f('bar');
}
