<?hh

function foo() {
  $a = 1;
  $b = 2;
  $c = 3;
  unset($a, $b, $c);
  var_dump($b);
}

<<__EntryPoint>>
function main_1113() {
foo();
}
