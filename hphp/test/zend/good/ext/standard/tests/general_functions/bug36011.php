<?hh

class TestClass
{
    <<__DynamicallyCallable>> static function test()
    {
        echo __METHOD__ . "()\n";
    }

    function whee()
    {
        array_map(varray['TestClass', 'test'], varray['array_value']);
    }

    function whee4()
    {
        call_user_func(varray['TestClass', 'test']);
    }

    static function whee5()
    {
        call_user_func(varray['TestClass', 'test']);
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
