<?hh

abstract class fail {
    abstract function show();
}

class pass extends fail {
    function show() {
        echo "Call to function show()\n";
    }
    function error() {
        parent::show();
    }
}
<<__EntryPoint>> function main(): void {
$t = new pass();
$t->show();
$t->error();

echo "Done\n"; // shouldn't be displayed
}
