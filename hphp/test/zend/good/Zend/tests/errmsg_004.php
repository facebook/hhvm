<?hh

function foo() :mixed{
    return "blah";
}
<<__EntryPoint>> function main(): void {
foo() = 1;

echo "Done\n";
}
