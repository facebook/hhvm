<?hh

class Test
{
    function __construct()
    {
        echo __METHOD__ . "()\n";
    }
}

class Derived extends Test
{
    function __construct()
    {
        echo __METHOD__ . "()\n";
        parent::__construct();
    }

    static function f()
    {
        new Derived;
    }
}

class TestPriv
{
    private function __construct()
    {
        echo __METHOD__ . "()\n";
    }

    static function f()
    {
        new TestPriv;
    }
}

class DerivedPriv extends TestPriv
{
    function __construct()
    {
        echo __METHOD__ . "()\n";
        parent::__construct();
    }

    static function f()
    {
        new DerivedPriv;
    }
}
<<__EntryPoint>> function main(): void {
Derived::f();

TestPriv::f();

DerivedPriv::f();

echo "===DONE===\n";
}
