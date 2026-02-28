<?hh

class TestClass
{
    <<__DynamicallyCallable>> static function test()
:mixed    {
        echo __METHOD__ . "()\n";
    }

    function whee()
:mixed    {
        array_map(vec['TestClass', 'test'], vec['array_value']);
    }

    function whee4()
:mixed    {
        call_user_func(vec['TestClass', 'test']);
    }

    static function whee5()
:mixed    {
        call_user_func(vec['TestClass', 'test']);
    }
}
<<__EntryPoint>> function main(): void {
TestClass::test();

$a = new TestClass();
$a->whee();
$a->whee4();
$a::whee5();

TestClass::whee5();

echo "===DONE===\n";
}
