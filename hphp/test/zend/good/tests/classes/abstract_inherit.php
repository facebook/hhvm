<?hh

abstract class pass {
    abstract function show():mixed;
}

abstract class fail extends pass {
}
<<__EntryPoint>> function main(): void {
$t = new fail();
$t = new pass();

echo "Done\n"; // Shouldn't be displayed
}
