<?hh

class test {
    function __call($name, inout $args) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$func = "foo";
$arg = 1;

$t->$func($arg);

echo "Done\n";
}
