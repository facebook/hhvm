<?hh

function f(...$args) {
  var_dump($args);
}
function g($x) {
  if ($x) $f = 'f';
  else    $f = '__nocall__';
  call_user_func_array($f,     darray['x' => 10, 'y' => 20, 'z' => 30, 'j' => 40]);
  call_user_func_array($f,     darray[3 => 10, 80 => 20, 10 => 30, 30 => 40]);
}

<<__EntryPoint>>
function main_26() {
g(10);
}
