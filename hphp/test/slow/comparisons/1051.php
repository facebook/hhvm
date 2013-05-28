<?php

function foo($p) {
  if ($p) {
    $a = 'foo';
  }
  if ('' < $a) {
    echo 'yes';
  }
 else {
    echo 'no';
  }
  if ($a > '') {
    echo 'yes';
  }
 else {
    echo 'no';
  }
}
foo(false);
