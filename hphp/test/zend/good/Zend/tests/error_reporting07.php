<?hh

function foo1($arg) :mixed{
}

function foo2($arg) :mixed{
}

function foo3($arg) :mixed{
    echo $undef3;
    throw new Exception("test");
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

try {
    @error_reporting(@foo1(@foo2(@foo3())));
} catch (Exception $e) {
}

var_dump(error_reporting());

echo "Done\n";
}
