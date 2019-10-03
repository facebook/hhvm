<?hh

class test {
    function __call(inout $name, $args) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$func = "foo";

$t->$func();

echo "Done\n";
}
