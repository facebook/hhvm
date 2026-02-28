<?hh

function f($val = (() ==> 42)()) :mixed{
  var_dump($val);
}


<<__EntryPoint>>
function main_short_lambda() :mixed{
f();
}
