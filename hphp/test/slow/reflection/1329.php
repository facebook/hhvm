<?php

class A {
 public static function test() {
 print 'ok';
}
}
var_dump(is_callable('A::test'));
var_dump(function_exists('A::test'));
