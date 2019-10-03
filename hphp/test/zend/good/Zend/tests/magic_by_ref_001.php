<?hh

class test {
    function __set(inout $name, $val) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$name = "prop";
$t->$name = 1;

echo "Done\n";
}
