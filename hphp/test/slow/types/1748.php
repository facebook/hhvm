<?hh

function foo($p) {
  if ($p) {
    $a = varray[];
  }
  var_dump((string)$a);
}

<<__EntryPoint>>
function main_1748() {
foo(false);
}
