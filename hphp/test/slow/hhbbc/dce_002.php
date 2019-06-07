<?hh

function asd() { return 12; }
function foo() {
  $x = asd();
  for ($i = 0; $i < 10; ++$i) { echo $i . "\n"; }
  $x += 2;
  echo $x;
  echo "\n";
}


<<__EntryPoint>>
function main_dce_002() {
foo();
}
