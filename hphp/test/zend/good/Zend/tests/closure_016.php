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
var_dump(is_callable_with_name($test, true, &$name));
echo $name."\n";
var_dump(is_callable_with_name($test, false, &$name));
echo $name."\n";
var_dump(is_callable_with_name(array($test,"__invoke"), true, &$name));
echo $name."\n";
var_dump(is_callable_with_name(array($test,"__invoke"), false, &$name));
echo $name."\n";
$test = foo();
var_dump(is_callable_with_name($test, true, &$name));
echo $name."\n";
var_dump(is_callable_with_name($test, false, &$name));
echo $name."\n";
var_dump(is_callable_with_name(array($test,"__invoke"), true, &$name));
echo $name."\n";
var_dump(is_callable_with_name(array($test,"__invoke"), false, &$name));
echo $name."\n";
}
