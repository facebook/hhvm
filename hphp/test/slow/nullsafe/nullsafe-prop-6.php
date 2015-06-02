<?hh // strict

function test() {
  $foo = null;
  echo 'not reached';
  $x =& $foo?->bar[1]; // parse error
  echo 'not reached';
}

test();
