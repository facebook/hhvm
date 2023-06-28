<?hh

function foo(callable $a, $b, callable $c) :mixed{
    var_dump($a, $b, $c);
}
function bar(callable $a = null) :mixed{
    var_dump($a);
}
<<__EntryPoint>> function main(): void {
foo("strpos", 123, "strpos");
bar("substr");
}
