<?hh

function f($b) :mixed{
  $a = $b ? 0 : dict['x' => $b];
  $a[0] = $a;
 var_dump($a);
}

<<__EntryPoint>>
function main_259() :mixed{
f(false);
}
