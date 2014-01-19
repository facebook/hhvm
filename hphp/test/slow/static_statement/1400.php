<?php

static $static_var = 1;
echo $static_var . "\n";
function test(){
  static $static_var = -1;
  echo $static_var . "\n";
  $static_var = 2;
  echo $static_var . "\n";
  $static_var++;
  echo $static_var . "\n";
}
test();
test();
