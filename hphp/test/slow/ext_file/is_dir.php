<?php
function test(){
  $baseDir = 'file://' . realpath(dirname(__FILE__));
  var_dump(is_dir($baseDir));
}
test();
