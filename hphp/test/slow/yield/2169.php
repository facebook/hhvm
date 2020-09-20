<?hh

function foo($x = null) {
  if ($x) $x = 'foo';
  var_dump($x);
  yield 1;
  }

<<__EntryPoint>>
function main_2169() {
foreach(foo() as $x) {
}
}
