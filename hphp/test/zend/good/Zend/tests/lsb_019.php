<?hh
class TestClass {
    protected static $staticVar;

    protected static function staticFunction() :mixed{
        return 'TestClassFunction';
    }

    public static function testStaticVar() :mixed{
        TestClass::$staticVar = 'TestClassStatic';
        ChildClass1::$staticVar = 'ChildClassStatic';
        return static::$staticVar;
    }

    public static function testStaticFunction() :mixed{
        return static::staticFunction();
    }
}

class ChildClass1 extends TestClass {
    public static $staticVar;

    public static function staticFunction() :mixed{
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
