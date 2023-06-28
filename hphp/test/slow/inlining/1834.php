<?hh

function bar($name) :mixed{
  $name = trim($name);
  var_dump($name);
}
function f($x, $y) :mixed{
 if ($x) return $x;
 return $y;
 }
function foo() :mixed{
  $name = 'ab.' . f('x', 'y');
  bar($name);
  bar($name);
}

<<__EntryPoint>>
function main_1834() :mixed{
foo();
}
