<?hh
class TestClass {
    protected static $staticVar;

    protected static function staticFunction() {
        return 'TestClassFunction';
    }

    public static function testStaticVar() {
        TestClass::$staticVar = 'TestClassStatic';
        ChildClass1::$staticVar = 'ChildClassStatic';
        return static::$staticVar;
    }

    public static function testStaticFunction() {
        return static::staticFunction();
    }
}

class ChildClass1 extends TestClass {
    public static $staticVar;

    public static function staticFunction() {
        return 'ChildClassFunction';
    }
}

class ChildClass2 extends TestClass {}
<<__EntryPoint>> function main(): void {
echo TestClass::testStaticVar() . "\n";
echo TestClass::testStaticFunction() . "\n";

echo ChildClass1::testStaticVar() . "\n";
echo ChildClass1::testStaticFunction() . "\n";

echo ChildClass2::testStaticVar() . "\n";
echo ChildClass2::testStaticFunction() . "\n";
}
