<?php

function thing_get() {
  return thing::get();
}

final class thing {
  function x() { echo "ok\n"; }

  public static function get() {
    static $instance = null;
    if ($instance === null) {
      $instance = new self();
    }
    return $instance;
  }
}

function go() {
  $z = thing_get();
  mt_rand();
  return $z->x();
}

go();
