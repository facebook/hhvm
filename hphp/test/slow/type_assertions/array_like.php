<?hh

function foo(KeyedTraversable<String> $foo, $f) {
  var_dump($foo);
}

function test($a) {
  if ($a instanceof KeyedTraversable) {
    foo($a, false);
  }
}

test(array(1));
