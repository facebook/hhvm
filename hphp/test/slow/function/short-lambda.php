<?hh

function f($val = (() ==> 42)()) {
  var_dump($val);
}

f();
