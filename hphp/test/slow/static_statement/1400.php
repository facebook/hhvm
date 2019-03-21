<?php
function test(){
  static $static_var = -1;
  echo $static_var . "\n";
  $static_var = 2;
  echo $static_var . "\n";
  $static_var++;
  echo $static_var . "\n";
}


<<__EntryPoint>>
function main_1400() {
static $static_var = 1;
echo $static_var . "\n";
test();
test();
}
