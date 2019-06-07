<?hh

final class base {
    function show() {
        echo "base\n";
    }
}

class derived extends base {
}
<<__EntryPoint>> function main() {
$t = new base();

echo "Done\n"; // shouldn't be displayed
}
