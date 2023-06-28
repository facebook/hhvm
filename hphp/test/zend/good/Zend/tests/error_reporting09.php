<?hh

function bar() :mixed{
    echo @$blah;
    echo $undef2;
}

function foo() :mixed{
    echo @$undef;
    error_reporting(E_ALL|E_STRICT);
    echo $blah;
    return bar();
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

@foo();

var_dump(error_reporting());

echo "Done\n";
}
