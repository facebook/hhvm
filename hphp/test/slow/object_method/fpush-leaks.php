<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class logger {
  private static $num;
  private $id;

  public function __construct() {
    $this->id = self::$num++;
    printf("logger %d constructing\n", $this->id);
  }
  public function __destruct() {
    printf("logger %d destructing\n", $this->id);
  }
}

function func($a, $b) {}

function main($o) {
  echo "about to call func\n";
  func(new logger, $o->foo());
  echo "leaving main\n";
}

main(new logger);
