<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class Bobble {
  public static function frob(string $thing) {
    return 'get';
  }
}
class BobbleProvider {
  private $type = 0;

  public function __call(string $type, $args = array()) {
    $name = Bobble::frob($this->type);
    if ($name !== $type) throw new Exception();
    return 0;
  }
}

$o = new BobbleProvider();
$o->wub('hello');
