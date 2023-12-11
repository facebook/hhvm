<?hh

function f(...$args) :mixed{
  var_dump($args);
}
function g($x) :mixed{
  if ($x) $f = f<>;
  else    $f = '__nocall__';
  call_user_func_array($f,     dict['x' => 10, 'y' => 20, 'z' => 30, 'j' => 40]);
  call_user_func_array($f,     dict[3 => 10, 80 => 20, 10 => 30, 30 => 40]);
}

<<__EntryPoint>>
function main_26() :mixed{
g(10);
}
