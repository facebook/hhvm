<?hh

interface Test
{
    function show():mixed;
}

class Tester extends Test
{
    function show() :mixed{
        echo __METHOD__ . "\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new Tester;
$o->show();

echo "===DONE===\n";
}
