<?php

class TestA {
protected static function doSomething() {
echo "TestA::doSomething\n";
}
protected static function test() {
static::doSomething();
}
public static function nativeTest($obj) {
$obj->bar();
self::test();
}
}
class Foo {
public function bar() {
}
}
$obj = new Foo();
TestA::nativeTest($obj);
