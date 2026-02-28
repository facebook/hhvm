<?hh

function foo(num $n): string {
  $x = 5 + $n;
  $x = 5 + $x;
  $x = $x + 5;
  $x = 5 + $x;
  $x = 5 + $x;
  return 5 + $x;
}
