<?hh // strict

function f() {
  $x = 1;
  $f = () ==> { return $x is int; };
  var_dump($f());
}

f();
