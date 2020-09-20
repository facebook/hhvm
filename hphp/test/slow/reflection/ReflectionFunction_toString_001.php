<?hh

function foo($bar, $baz = 123) {}


<<__EntryPoint>>
function main_reflection_function_to_string_001() {
echo (string)(new ReflectionFunction('foo'));
}
