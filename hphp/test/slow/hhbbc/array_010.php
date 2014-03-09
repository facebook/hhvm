<?php

function foo() {
  $x = null;
  for ($i = 0; $i < 1024; ++$i) {
    $x = array('foo' => $x);
  }
}
foo();
