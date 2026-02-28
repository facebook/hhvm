<?hh

function block() :mixed{
}
function f($x) :mixed{
  if (is_int($x) || is_array($x)) {
    var_dump($x[0]);
  }
}
function g($x) :mixed{
  $x = darray($x);
  block();
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_1858() :mixed{
f(vec[10]);
g(vec[10]);
}
