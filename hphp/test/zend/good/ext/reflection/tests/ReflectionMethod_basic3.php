<?hh

function reflectMethod($class, $method) :mixed{
    $methodInfo = new ReflectionMethod($class, $method);
    echo "**********************************\n";
    echo "Reflecting on method $class::$method()\n\n";
    echo "\ngetName():\n";
    var_dump($methodInfo->getName());
    echo "\nisInternal():\n";
    var_dump($methodInfo->isInternal());
    echo "\nisUserDefined():\n";
    var_dump($methodInfo->isUserDefined());
    echo "\n**********************************\n";
}

class TestClass
{
    public function foo() :mixed{
        echo "Called foo()\n";
    }

    static function stat() :mixed{
        echo "Called stat()\n";
    }

    private function priv() :mixed{
        echo "Called priv()\n";
    }

    protected function prot() :mixed{}
}

class DerivedClass extends TestClass {}

interface TestInterface {
    public function int():mixed;
}
<<__EntryPoint>> function main(): void {
reflectMethod("DerivedClass", "foo");
reflectMethod("TestClass", "stat");
reflectMethod("TestClass", "priv");
reflectMethod("TestClass", "prot");
reflectMethod("DerivedClass", "prot");
reflectMethod("TestInterface", "int");
reflectMethod("ReflectionProperty", "__construct");
}
