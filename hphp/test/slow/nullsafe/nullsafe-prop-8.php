<?hh // strict

function test() {
  $foo = null;
  echo 'not reached';
  $x =& $foo?->bar; // emit error
  echo 'not reached';
}

test();
