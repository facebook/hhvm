<?hh
class Foo {
}

function blah (Foo $a)
:mixed{
}

function error()
:mixed{
    $a = func_get_args();
    var_dump($a);
}
<<__EntryPoint>> function main(): void {
blah (new stdClass);
echo "ALIVE!\n";
}
