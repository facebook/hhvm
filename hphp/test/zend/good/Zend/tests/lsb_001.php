<?hh

class TestClass {
    protected static $staticVar = 'TestClassStatic';
    const CLASS_CONST = 'TestClassConst';

    protected static function staticFunction() :mixed{
        return 'TestClassFunction';
    }

    public static function testStaticVar() :mixed{
        return static::$staticVar;
    }

    public static function testClassConst() :mixed{
        return static::CLASS_CONST;
    }

    public static function testStaticFunction() :mixed{
        return static::staticFunction();
    }
}

class ChildClass1 extends TestClass {
    protected static $staticVar = 'ChildClassStatic';
    const CLASS_CONST = 'ChildClassConst';

    protected static function staticFunction() :mixed{
        return 'ChildClassFunction';
    }
}

class ChildClass2 extends TestClass {}
<<__EntryPoint>> function main(): void {
echo TestClass::testStaticVar() . "\n";
echo TestClass::testClassConst() . "\n";
echo TestClass::testStaticFunction() . "\n";

echo ChildClass1::testStaticVar() . "\n";
echo ChildClass1::testClassConst() . "\n";
echo ChildClass1::testStaticFunction() . "\n";

echo ChildClass2::testStaticVar() . "\n";
echo ChildClass2::testClassConst() . "\n";
echo ChildClass2::testStaticFunction() . "\n";
echo "==DONE==";
}
