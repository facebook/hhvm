<?php

class test extends SplFileInfo {
  public function example() {
    return "From class test";
  }
}

$info = new SplFileInfo(__FILE__);
$info->setInfoClass('test');
var_dump($info->getFileInfo()->example());
var_dump($info->getPathInfo()->example());

try {
  $info->setInfoClass('stdclass');
} catch (UnexpectedValueException $e) {
  echo $e->getMessage(), PHP_EOL;
}
