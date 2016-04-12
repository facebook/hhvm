<?php

function y() {
  static $x = 0;
  echo "warn\n";
  if ($x++ == 1) {
    throw new exception('z');
  }
}
set_error_handler('y');

class ZZ {
  private $asd;

  function __construct() {
    unset($this->asd);
  }

  function __get($z) {
    echo "get\n";
    return new stdclass;
  }

  static function x(ZZ $x) {
    try {
      $x->asd->asd->asd->asd->asd->asd->asd = 2;
      echo "ok\n";
    } catch (exception $z) {
      var_dump($x);
    }
  }
}

ZZ::x(new ZZ);
