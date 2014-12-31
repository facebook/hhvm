<?php

class test extends SplFileObject {
  public function example() {
    return "From class test";
  }
}

$info = new SplFileInfo(__FILE__);
$info->setFileClass('test');
var_dump($info->openFile()->example());

try {
  $info->setFileClass('stdclass');
} catch (UnexpectedValueException $e) {
  echo $e->getMessage(), PHP_EOL;
}
