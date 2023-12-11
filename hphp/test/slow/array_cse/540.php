<?hh

function foo($a, $b, $c, $d) :mixed{
  var_dump($a, $b, $c, $d);
}
function f($a) :mixed{
  foo($a[0], $a[0], $a[0], $a[0]++);
  foo($a[0], $a[0], $a[0], $a[0]);
}
<<__EntryPoint>>
function main_540() :mixed{
f(vec[0]);
}
