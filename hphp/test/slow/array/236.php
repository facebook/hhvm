<?hh

function foo() {
  $a = varray[];
  $a[] = '1.1';
  $a[] = '2.2';
  $a[] = '3.3';
  var_dump(array_sum($a));
  var_dump(array_product($a));
}

<<__EntryPoint>>
function main_236() {
foo();
}
