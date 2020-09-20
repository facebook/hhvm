<?hh

abstract class test_base
{
    abstract function func();
}

class test extends test_base
{
    function func()
    {
        echo __METHOD__ . "()\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new test;

$o->func();

call_user_func(varray[$o, 'test_base::func']);

echo "===DONE===\n";
}
