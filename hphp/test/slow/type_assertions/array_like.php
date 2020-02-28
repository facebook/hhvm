<?hh

function foo1(KeyedTraversable<string> $foo, $f) {
  var_dump($foo);
}

function foo2(KeyedContainer<string> $foo, $f) {
  var_dump($foo);
}

function test($a) {
  if ($a is KeyedTraversable) {
    foo1($a, false);
  }
  if ($a is KeyedContainer) {
    foo2($a, false);
  }
}


<<__EntryPoint>>
function main_array_like() {
test(varray[1]);
}
