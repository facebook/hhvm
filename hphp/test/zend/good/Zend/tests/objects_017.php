<?hh

class foo {
    private $test = 1;
}

function test() {
    return new foo;
}
<<__EntryPoint>> function main(): void {
test()->test = 2;
}
