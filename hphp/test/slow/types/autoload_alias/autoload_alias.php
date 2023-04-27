<?hh

function foo(MyVector $a) {
  var_dump($a);
}


<<__EntryPoint>>
function main_autoload_alias() {
  foo(Vector { });
}
