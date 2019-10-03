<?hh

class test {
    function __isset(inout $name) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$name = "prop";

var_dump(isset($t->$name));

echo "Done\n";
}
