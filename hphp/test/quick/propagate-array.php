<?hh

function foo(inout $x, $y) :mixed{
  $x = vec[1,2];
  $y = $x;
  return $y;
}
<<__EntryPoint>> function main(): void {
$x = 0;
$y = 0;
var_dump(foo(inout $x, $y));
var_dump($x);
}
