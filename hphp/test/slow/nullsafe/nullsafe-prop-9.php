<?hh // strict

function test() {
  $foo = null;
  echo 'not reached';
  $x =& $foo->bar?->baz; // emit error
  echo 'not reached';
}

test();
