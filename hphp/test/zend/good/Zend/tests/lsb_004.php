<?hh

class TestClass {
    public static function getClassName() :mixed{
        return static::class;
    }
}

class ChildClass extends TestClass {}
<<__EntryPoint>> function main(): void {
echo TestClass::getClassName() . "\n";
echo ChildClass::getClassName() . "\n";
echo "==DONE==";
}
