<?hh

function foo($bar, $baz = 123) :mixed{}


<<__EntryPoint>>
function main_reflection_function_to_string_001() :mixed{
echo (string)(new ReflectionFunction('foo'));
}
