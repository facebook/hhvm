<?php

class Test{
  protected static $color = array('gray' => 30);
  public static function foo($type, $key) {
    return isset( self::${
$type}
[$key] );
  }
}
var_dump(Test::foo('color', 'gray'));
