<?hh

class foo {
    private $test = 1;
}

function test() :mixed{
    return new foo;
}
<<__EntryPoint>> function main(): void {
test()->test = 2;
}
