<?hh

function foo(bool $x) :mixed{
  return darray['flag' => darray['flag2' => $x]];
}
function bar() :mixed{
  $x = foo(true);
  $y = foo(false);
  $z = $x['flag']['flag2'];
  $l = $y['flag']['flag2'];
  var_dump($z, $l);
}

<<__EntryPoint>>
function main_array_013() :mixed{
bar();
}
