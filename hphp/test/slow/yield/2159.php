<?hh

function f($x) {
  switch ($x++ + ++$x) {
  case 1:
    yield 1;
  case 2:
    yield 2;
  case 3:
    yield 3;
  case 4:
    yield 4;
  }
}

<<__EntryPoint>>
function main_2159() {
foreach (f(0) as $x) {
 var_dump($x);
 }
foreach (f(1) as $x) {
 var_dump($x);
 }
}
