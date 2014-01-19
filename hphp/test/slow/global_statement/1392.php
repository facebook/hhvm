<?php

function test() {
  if (true) {
    global $a;
    $a = 10;
  }
}
test();
var_dump($a);
