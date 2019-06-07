<?hh

interface Test
{
    function show();
}

class Tester extends Test
{
    function show() {
        echo __METHOD__ . "\n";
    }
}
<<__EntryPoint>> function main() {
$o = new Tester;
$o->show();

echo "===DONE===\n";
}
