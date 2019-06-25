<?hh

class test {
    function __call(&$name, $args) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$func = "foo";

$t->$func();

echo "Done\n";
}
