<?php

class A {
  const C = 123;
  static public $foo = 456;
  public function bar() {
    return 789;
  }
}
$cls = 'a';

var_dump($cls::C);
 // ClassConstant

var_dump($cls::$foo);
 // StaticMember
$cls::$foo = 'test';
var_dump($cls::$foo);
 // l-value

var_dump($cls::bar());
 // SimpleFunctionCall

$func = 'bar';
var_dump($cls::$func());
 // DynamicFunctionCall
