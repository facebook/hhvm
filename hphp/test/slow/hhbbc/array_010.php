<?hh

function foo() {
  $x = null;
  for ($i = 0; $i < 1024; ++$i) {
    $x = darray['foo' => $x];
  }
}

<<__EntryPoint>>
function main_array_010() {
foo();
}
