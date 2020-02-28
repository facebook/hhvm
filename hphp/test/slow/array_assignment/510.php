<?hh

function foo() {
  $p = 1;
  $q = 2;
  $r = 3;
  $s = 4;
  $a = darray['1'=>$p, '2'=>$q];
  $b = darray['3'=>$r, '4'=>$s];
  var_dump($a);
  $a += $b;
  var_dump($a);
  var_dump($b);
}

<<__EntryPoint>>
function main_510() {
foo();
}
