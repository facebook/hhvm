<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class logger {
  private $n;
  private static $count = 0;

  function __construct() {
    $this->n = self::$count++;
    printf("logger %d constructing\n", $this->n);
  }

  function __destruct() {
    printf("logger %d destructing\n", $this->n);
  }
}

class MagicThing implements ArrayAccess {
  private $logger;

  function __construct() {
    $this->logger = new logger;
  }

  function &__get($name) {
    return $this->logger;
  }

  function offsetExists($offset) { return true; }
  function &offsetGet($offset) {
    return $this->logger;
  }
  function offsetSet($offset, $value) {
  }
  function offsetUnset($offset) {
  }
}

function main() {
  echo "Starting main\n";
  $o = new MagicThing;
  $o->prop;
  $o = new MagicThing;
  $o['elem'];
  $o = null;
  echo "Leaving main\n";
}

main();
echo "Done\n";
