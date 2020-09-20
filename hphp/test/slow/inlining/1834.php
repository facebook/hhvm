<?hh

function bar($name) {
  $name = trim($name);
  var_dump($name);
}
function f($x, $y) {
 if ($x) return $x;
 return $y;
 }
function foo() {
  $name = 'ab.' . f('x', 'y');
  bar($name);
  bar($name);
}

<<__EntryPoint>>
function main_1834() {
foo();
}
