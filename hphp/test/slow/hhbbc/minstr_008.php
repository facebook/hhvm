<?hh

function asd() { return 'xxxxx'; }
function foo() {
  $x = asd();
  $x[0] = 'a';
  echo $x;
  echo "\n";
  var_dump($x);
}

<<__EntryPoint>>
function main_minstr_008() {
foo();
}
