<?hh // strict

function f() {
  $x = 1;
  $f = () ==> { return $x is int; };
  var_dump($f());
}


<<__EntryPoint>>
function main_is_expression_in_closure() {
f();
}
