<?hh

function lol() :mixed{ return HH\stdin(); }
function foo() :mixed{
  $x = lol();
  unset($x[0]['id']);
  return $x;
}


<<__EntryPoint>>
function main_elemu() :mixed{
error_reporting(-1);

var_dump(foo());
}
