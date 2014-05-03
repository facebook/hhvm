<?hh

function foo1(KeyedTraversable<string> $foo, $f) {
  var_dump($foo);
}

function foo2(KeyedContainer<string> $foo, $f) {
  var_dump($foo);
}

function test($a) {
  if ($a instanceof KeyedTraversable) {
    foo1($a, false);
  }
  if ($a instanceof KeyedContainer) {
    foo2($a, false);
  }
}

test(array(1));
