<?hh

class fail {
    final abstract function show():mixed;
}
<<__EntryPoint>> function main(): void {
echo "Done\n"; // Shouldn't be displayed
}
