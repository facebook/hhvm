<?hh

function foo($p) {
  $a = varray['a', 'b', $p];
  $a[] = 'd';
  var_dump($a);
  $a = darray[0 => 'a', 1 => 'b', 2 => $p];
  $a[] = 'd';
  var_dump($a);
  $a = darray[2 => 'a', 4 => 'b', 6 => $p];
  $a[] = 'd';
  var_dump($a);
  $a = darray[-2 => 'a', -4 => 'b', -6 => $p];
  $a[] = 'd';
  var_dump($a);
  $a = darray[0 => 'a'];
  $a[] = 'b';
  var_dump($a);
}

<<__EntryPoint>>
function main_248() {
foo('c');
}
