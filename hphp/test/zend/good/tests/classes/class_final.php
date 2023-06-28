<?hh

final class base {
    function show() :mixed{
        echo "base\n";
    }
}

class derived extends base {
}
<<__EntryPoint>> function main(): void {
$t = new base();

echo "Done\n"; // shouldn't be displayed
}
