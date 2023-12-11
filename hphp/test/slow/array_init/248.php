<?hh

function foo($p) :mixed{
  $a = vec['a', 'b', $p];
  $a[] = 'd';
  var_dump($a);
  $a = dict[0 => 'a', 1 => 'b', 2 => $p];
  $a[] = 'd';
  var_dump($a);
  $a = dict[2 => 'a', 4 => 'b', 6 => $p];
  $a[] = 'd';
  var_dump($a);
  $a = dict[-2 => 'a', -4 => 'b', -6 => $p];
  $a[] = 'd';
  var_dump($a);
  $a = dict[0 => 'a'];
  $a[] = 'b';
  var_dump($a);
}

<<__EntryPoint>>
function main_248() :mixed{
foo('c');
}
