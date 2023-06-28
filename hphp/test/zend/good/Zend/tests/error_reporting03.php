<?hh

function foo($arg) :mixed{
    echo @$nonex_foo;
}

function bar() :mixed{
    echo @$nonex_bar;
    throw new Exception("test");
}

function foo1() :mixed{
    echo $undef1;
    error_reporting(E_ALL|E_STRICT);
    echo $undef2;
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

try {
    @foo(@bar(@foo1()));
} catch (Exception $e) {
    var_dump($e->getMessage());
}

var_dump(error_reporting());

echo "Done\n";
}
