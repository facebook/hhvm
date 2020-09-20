<?hh

function f($val = (() ==> 42)()) {
  var_dump($val);
}


<<__EntryPoint>>
function main_short_lambda() {
f();
}
