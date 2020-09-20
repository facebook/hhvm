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
$name = null;
$test = new Foo;
var_dump(is_callable_with_name($test, true, inout $name));
echo $name."\n";
var_dump(is_callable_with_name($test, false, inout $name));
echo $name."\n";
var_dump(is_callable_with_name(varray[$test,"__invoke"], true, inout $name));
echo $name."\n";
var_dump(is_callable_with_name(varray[$test,"__invoke"], false, inout $name));
echo $name."\n";
$test = foo();
var_dump(is_callable_with_name($test, true, inout $name));
echo $name."\n";
var_dump(is_callable_with_name($test, false, inout $name));
echo $name."\n";
var_dump(is_callable_with_name(varray[$test,"__invoke"], true, inout $name));
echo $name."\n";
var_dump(is_callable_with_name(varray[$test,"__invoke"], false, inout $name));
echo $name."\n";
}
