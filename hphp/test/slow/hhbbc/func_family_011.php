<?php

abstract class Unimpl {
  static abstract function aa($x);

  static function go() {
    return self::doweirdthings();
  }

  static function doweirdthings() {
    $k = array();
    return static::aa($k);
  }
}

abstract class B extends Unimpl {
  static function aa($x) {
    return "hi\n";
  }

  static abstract function bb($x);

  static function doit() {
    parent::go();
  }
}

B::doit("asd");
