<?hh

function foo($arg) {
    echo @$nonex_foo;
}

function bar() {
    echo @$nonex_bar;
    throw new Exception("test");
}

function foo1() {
    echo $undef1;
    error_reporting(E_ALL|E_STRICT);
    echo $undef2;
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

try {
    @foo(@bar(@foo1()));
} catch (Exception $e) {
}

var_dump(error_reporting());

echo "Done\n";
}
