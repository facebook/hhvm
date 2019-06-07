<?hh

class pass {
    final function show() {
        echo "Call to function pass::show()\n";
    }
}

class fail extends pass {
    function show() {
        echo "Call to function fail::show()\n";
    }
}

<<__EntryPoint>> function main() {
$t = new pass();

echo "Done\n"; // Shouldn't be displayed
}
