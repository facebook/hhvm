<?hh

class pass {
    function show() {
        echo "Call to function show()\n";
    }
}

class fail extends pass {
    abstract function show();
}
<<__EntryPoint>> function main(): void {
echo "Done\n"; // Shouldn't be displayed
}
