<?hh
function foo() :mixed{
  $x = Vector {1, 2};
  $y = $x->immutable();
  $a = $x->toVArray();
  $b = $y->toVArray();
  var_dump($a, $b);
  $a[] = 3;
  $b[] = 4;
  var_dump($a, $b);
  var_dump($x, $y);
}

<<__EntryPoint>>
function main_to_array() :mixed{
foo();
}
