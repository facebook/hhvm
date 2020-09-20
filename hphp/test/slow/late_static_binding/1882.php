<?hh

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

<<__EntryPoint>>
function main_1882() {
$obj = new Foo();
TestA::nativeTest($obj);
}
