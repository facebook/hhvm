<?hh

abstract class fail {
    abstract function show():mixed;
}

class pass extends fail {
    function show() :mixed{
        echo "Call to function show()\n";
    }
    function error() :mixed{
        parent::show();
    }
}
<<__EntryPoint>> function main(): void {
$t = new pass();
$t->show();
$t->error();

echo "Done\n"; // shouldn't be displayed
}
