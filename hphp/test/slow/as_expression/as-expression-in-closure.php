<?hh // strict

function f() {
  $x = 1;
  $f = () ==> { return $x as int; };
  var_dump($f());
}

f();
