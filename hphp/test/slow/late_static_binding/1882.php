<?hh

class TestA {
protected static function doSomething() :mixed{
echo "TestA::doSomething\n";
}
protected static function test() :mixed{
static::doSomething();
}
public static function nativeTest($obj) :mixed{
$obj->bar();
self::test();
}
}
class Foo {
public function bar() :mixed{
}
}

<<__EntryPoint>>
function main_1882() :mixed{
$obj = new Foo();
TestA::nativeTest($obj);
}
