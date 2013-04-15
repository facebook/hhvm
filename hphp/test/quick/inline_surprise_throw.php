<?php

# bug #2088495
function asd($x, $y) {
  if ($x == 'exit' && $y == 'foo') {
    echo "yep\n";
    throw new Exception ('yo');
  }
  echo "hi $x $y\n";
}
fb_setprofile('asd');

function foo() {
  $x = new stdclass;
}

try {
  foo();
} catch (Exception $x) {
  echo $x->getMessage() . "\n";
}
