<?php

abstract class class1 {
  public function getConstViaThis() {
    return defined('static::SOME_CONST') ? static::SOME_CONST : false;
  }
  public static function getConstViaFrame() {
    return defined('static::SOME_CONST') ? static::SOME_CONST : false;
  }
}

final class class2 extends class1 {
  const SOME_CONST = 2;
}

$class2 = new class2;

var_dump($class2->getConstViaThis());
var_dump($class2->getConstViaFrame());
