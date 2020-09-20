<?hh
class Foo {
    function __invoke() {
        echo "Hello World!\n";
    }
}

function foo() {
    return function() {
        echo "Hello World!\n";
    };
}
<<__EntryPoint>> function main(): void {
$test = new Foo;
$test->__invoke();
$test = foo();
$test->__invoke();
$test = foo()->__invoke();
}
