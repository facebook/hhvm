<?php

function test() {
  $f = fopen(__FILE__, "r");
  var_export($f);
  echo "\n";
  var_dump(var_export($f, true));
}

test();
