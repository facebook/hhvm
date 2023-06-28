<?hh

trait TestTrait {
    public static function test() :mixed{
        return static::$test;
    }
}

class A {
    use TestTrait;
    protected static $test = "Test A";
}

class B extends A {
    protected static $test = "Test B";
}
<<__EntryPoint>> function main(): void {
echo B::test();
}
