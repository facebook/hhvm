<?hh

abstract class fail {
    abstract function show():mixed;
}

class pass extends fail {
    function show() :mixed{
        echo "Call to function show()\n";
    }
}
<<__EntryPoint>> function main(): void {
$t2 = new pass();
$t2->show();

$t = new fail();
$t->show();

echo "Done\n"; // shouldn't be displayed
}
