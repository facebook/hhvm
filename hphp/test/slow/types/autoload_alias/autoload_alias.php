<?hh

function foo(MyVector $a) :mixed{
  var_dump($a);
}


<<__EntryPoint>>
function main_autoload_alias() :mixed{
  foo(Vector { });
}
