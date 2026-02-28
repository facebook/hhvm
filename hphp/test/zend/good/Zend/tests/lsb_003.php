<?hh

class TestClass {
    public static function createInstance() :mixed{
        return new static();
    }
}

class ChildClass extends TestClass {}
<<__EntryPoint>> function main(): void {
$testClass = TestClass::createInstance();
$childClass = ChildClass::createInstance();

echo get_class($testClass) . "\n";
echo get_class($childClass) . "\n";
echo "==DONE==";
}
