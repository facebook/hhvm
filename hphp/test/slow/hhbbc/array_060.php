<?hh

function foo() {
  $x = varray[1];
  for ($i = 0; $i < 2; ++$i) {
    $x[$i] = 'a';
  }
  if ($x ?? false) {
    echo "not empty\n";
  } else {
    echo "hm\n";
  }
}


<<__EntryPoint>>
function main_array_060() {
foo();
}
