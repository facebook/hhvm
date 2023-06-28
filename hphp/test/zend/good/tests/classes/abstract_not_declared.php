<?hh

class fail {
    abstract function show():mixed;
}
<<__EntryPoint>> function main(): void {
echo "Done\n"; // shouldn't be displayed
}
