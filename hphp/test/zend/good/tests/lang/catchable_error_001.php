<?hh
class Foo {
}

function blah (Foo $a)
{
}

function error()
{
    $a = func_get_args();
    var_dump($a);
}
<<__EntryPoint>> function main(): void {
blah (new stdClass);
echo "ALIVE!\n";
}
