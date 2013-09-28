<?php

class X {
  static function test() {
    var_dump(__FUNCTION__);
    var_dump(__CLASS__);
    var_dump(__METHOD__);
    return array($GLOBALS[__FUNCTION__],                 $GLOBALS[__CLASS__],                 $GLOBALS[__METHOD__]);
  }
}
$test = 'this_is_function_test';
$X = 'this_is_class_x';
$GLOBALS['X::test'] = 'this_is_method_test::x';
var_dump(X::test());
