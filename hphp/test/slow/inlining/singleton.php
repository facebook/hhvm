<?php

/**
 * Class with two singleton instances.
 */
class C {
  private static $instanceSame = null;
  private static $instanceNSame = null;

  public function __construct() {
    echo "C!\n";
  }

  public static function getSame() {
    if (self::$instanceSame === null) {
      echo __FUNCTION__ . " made a ";
      self::$instanceSame = new C();
    }
    return self::$instanceSame;
  }

  public static function getNSame() {
    if (self::$instanceNSame !== null) {
      return self::$instanceNSame;
    }
    echo __FUNCTION__ . " made a ";
    self::$instanceNSame = new C();
    return self::$instanceNSame;
  }
}

/**
 * Toplevel functions with static local singletons.
 *
 * The *_loop functions are the same as the normal versions; they're duplicated
 * only to get fresh the static locals.
 */
function check_null_same() {
  static $instance = null;
  if ($instance === null) {
    echo __FUNCTION__ . " made a ";
    $instance = new C;
  }
  return $instance;
}

function check_null_same_loop() {
  static $instance = null;
  if ($instance === null) {
    echo __FUNCTION__ . " made a ";
    $instance = new C;
  }
  return $instance;
}

function check_null_nsame() {
  static $instance = null;
  if ($instance !== null) {
    return $instance;
  }
  echo __FUNCTION__ . " made a ";
  $instance = new C;
  return $instance;
}

function check_null_nsame_loop() {
  static $instance = null;
  if ($instance !== null) {
    return $instance;
  }
  echo __FUNCTION__ . " made a ";
  $instance = new C;
  return $instance;
}

/**
 * Tests.
 */
function run_null_same() {
  var_dump(check_null_same());
  var_dump(check_null_same());
}

function run_null_same_loop() {
  for ($i = 0; $i < 2; ++$i) {
    var_dump(check_null_same_loop());
  }
}

function run_null_nsame() {
  var_dump(check_null_nsame());
  var_dump(check_null_nsame());
}

function run_null_nsame_loop() {
  for ($i = 0; $i < 2; ++$i) {
    var_dump(check_null_nsame_loop());
  }
}

function run_sprop_get_same() {
  var_dump(C::getSame());
  var_dump(C::getSame());
}

function run_sprop_get_nsame() {
  var_dump(C::getNSame());
  var_dump(C::getNSame());
}

run_null_same();
run_null_same_loop();
run_null_nsame();
run_null_nsame_loop();
run_sprop_get_same();
run_sprop_get_nsame();
