<?hh

function f(
  (function (int): bool) $func = (function(int $x): bool { return $x == 123; })
) :mixed{
  echo "func(123) == ";
  var_dump($func(123));
  echo "func(124) == ";
  var_dump($func(124));
}

function g((function (int): bool) $func = ($x ==> $x == 123)) :mixed{
  echo "func(123) == ";
  var_dump($func(123));
  echo "func(124) == ";
  var_dump($func(124));
}


<<__EntryPoint>>
function main_closure_default() :mixed{
f();
f($x ==> $x == 124);

g();
g(function (int $x) { return $x == 124; });
}
