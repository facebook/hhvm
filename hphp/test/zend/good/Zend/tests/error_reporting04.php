<?hh

function foo() :mixed{
    echo $undef;
    error_reporting(E_ALL|E_STRICT);
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

foo(@$var);

var_dump(error_reporting());

echo "Done\n";
}
