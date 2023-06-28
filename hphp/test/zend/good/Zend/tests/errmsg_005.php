<?hh

class test {
    function foo() :mixed{
        return "blah";
    }
}
<<__EntryPoint>> function main(): void {
$t = new test;
$t->foo() = 1;

echo "Done\n";
}
