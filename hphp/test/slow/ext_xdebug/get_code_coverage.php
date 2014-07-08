<?php
xdebug_start_code_coverage();

function a($a) {
  var_dump($a * 2.5);
}

function b($count) {
  for ($i = 0; $i < $count; $i++) {
    a($i + 0.17);
  }
}

b(6);
b(10);

var_dump(xdebug_get_code_coverage());
