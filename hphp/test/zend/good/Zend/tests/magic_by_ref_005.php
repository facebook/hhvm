<?hh

class test {
    function __isset(&$name) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$name = "prop";

var_dump(isset($t->$name));

echo "Done\n";
}
