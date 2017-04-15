<?php
function test(){
  $baseDir = 'file://' . realpath(dirname(__FILE__));
  var_dump(is_dir($baseDir));
  $fileName= 'file://' . realpath(__FILE__);
  var_dump(file_exists($fileName));
}
test();

