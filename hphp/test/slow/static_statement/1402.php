<?php

function test() {
  if (false) {
    static $static_var = +3;
  }
  var_dump($static_var);
  $static_var = 4;
  var_dump($static_var);
}
test();
