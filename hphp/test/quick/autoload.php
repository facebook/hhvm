<?php
function __autoload($cls) {
  echo "__autoload $cls\n";
  if ($cls === 'C') {
    class C { public static function foo() { } }
  }
}
function my_autoload_func($cls) {
  echo "my_autoload_func $cls\n";
  if ($cls === 'D') {
    class D { public static function foo() { } }
  }
}

var_dump(is_callable(array('D', 'foo')));
spl_autoload_register('my_autoload_func');
var_dump(is_callable(array('D', 'foo')));

var_dump(is_callable(array('C', 'foo')));
spl_autoload_register('__autoload');
var_dump(is_callable(array('C', 'foo')));

