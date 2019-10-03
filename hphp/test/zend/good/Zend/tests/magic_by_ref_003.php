<?hh

class test {
    function __get(inout $name) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$name = "prop";
var_dump($t->$name);

echo "Done\n";
}
