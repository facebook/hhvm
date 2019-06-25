<?hh

function foo($arg) {
}

function bar() {
    throw new Exception("test");
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

try {
    @foo(@bar());
} catch (Exception $e) {
}

var_dump(error_reporting());

echo "Done\n";
}
