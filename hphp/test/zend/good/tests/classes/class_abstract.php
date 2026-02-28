<?hh

abstract class base {
    function show() :mixed{
        echo "base\n";
    }
}

class derived extends base {
}
<<__EntryPoint>> function main(): void {
$t = new derived();
$t->show();

$t = new base();
$t->show();

echo "Done\n"; // shouldn't be displayed
}
