<?hh

abstract class test_base
{
    abstract function func():mixed;
}

class test extends test_base
{
    function func()
:mixed    {
        echo __METHOD__ . "()\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new test;

$o->func();

call_user_func(vec[$o, 'test_base::func']);

echo "===DONE===\n";
}
