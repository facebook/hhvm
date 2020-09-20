<?hh

function asd() { return varray[]; }
function foo() {
  $x = asd();
  foreach ($x as $k => $v) {
    echo "unreachable code: $k $v\n";
  }
  return 1;
}

<<__EntryPoint>>
function main_iter_004() {
foo();
}
