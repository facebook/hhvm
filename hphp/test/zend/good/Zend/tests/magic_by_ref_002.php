<?hh

class test {
    function __set($name, inout $val) { }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$t->prop = 1;

echo "Done\n";
}
