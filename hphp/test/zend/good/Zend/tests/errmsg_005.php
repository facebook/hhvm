<?hh

class test {
    function foo() {
        return "blah";
    }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$t->foo() = 1;

echo "Done\n";
}
