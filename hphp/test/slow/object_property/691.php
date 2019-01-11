<?php
class X {
  const UNKNOWN = 1;
  public $foo = -1;
  static public $bar = FOO;
  public $baz = self::UNKNOWN;
}


<<__EntryPoint>>
function main_691() {
define('FOO', 'foo');
$vars = get_class_vars('X');
 asort(&$vars);
 var_dump($vars);
}
