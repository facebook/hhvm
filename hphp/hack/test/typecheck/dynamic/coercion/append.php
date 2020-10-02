<?hh // strict

function foo(varray<dynamic> $a): varray<dynamic> {
  $a[] = 5;
  $a[0] = "hi";
  return $a;
}
