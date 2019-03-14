<?php

abstract final class YStatics {
  public static $x = 0;
}

function y() {
  echo "warn\n";
  if (YStatics::$x++ == 1) {
    throw new exception('z');
  }
}

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

<<__EntryPoint>>
function main_override_magic2() {
set_error_handler('y');

ZZ::x(new ZZ);
}
