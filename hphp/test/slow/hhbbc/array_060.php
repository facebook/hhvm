<?php

function foo() {
  $x = array(1);
  for ($i = 0; $i < 2; ++$i) {
    $x[$i] = 'a';
  }
  if (!empty($x)) {
    echo "not empty\n";
  } else {
    echo "hm\n";
  }
}

foo();
