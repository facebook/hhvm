<?hh

class pass {
    function show() :mixed{
        echo "Call to function show()\n";
    }
}

class fail extends pass {
    abstract function show():mixed;
}
<<__EntryPoint>> function main(): void {
echo "Done\n"; // Shouldn't be displayed
}
