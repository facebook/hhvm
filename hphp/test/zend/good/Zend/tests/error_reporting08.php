<?hh

function foo1($arg) :mixed{
}

function foo2($arg) :mixed{
}

function foo3($arg) :mixed{
    error_reporting(E_ALL|E_STRICT);
    echo $undef3;
    throw new Exception("test");
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

try {
    @foo1(@foo2(@foo3(1)));
} catch (Exception $e) {
    var_dump($e->getMessage());
}

var_dump(error_reporting());

echo "Done\n";
}
