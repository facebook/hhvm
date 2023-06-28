<?hh

function foo($arg) :mixed{
}

function bar() :mixed{
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
