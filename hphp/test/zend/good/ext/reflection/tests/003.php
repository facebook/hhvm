<?hh

class Foo
{
    function Test()
    {
        echo __METHOD__ . "\n";
    }
}

class Bar extends Foo
{
    function Test()
    {
        echo __METHOD__ . "\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new Bar;
$r = new ReflectionMethod('Foo','Test');

$r->invoke($o);

echo "===DONE===\n";
}
