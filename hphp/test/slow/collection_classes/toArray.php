<?hh
function foo() {
  $x = Vector {1, 2};
  $y = $x->immutable();
  $a = $x->toArray();
  $b = $y->toArray();
  var_dump($a, $b);
  $a[] = 3;
  $b[] = 4;
  var_dump($a, $b);
  var_dump($x, $y);
}
foo();
