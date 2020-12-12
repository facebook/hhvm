<?hh // strict

function f() {
  $x = 1;
  $f = () ==> { return $x as int; };
  var_dump($f());
}


<<__EntryPoint>>
function main_as_expression_in_closure() {
f();
}
