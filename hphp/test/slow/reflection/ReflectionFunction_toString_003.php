<?hh

function zeroArgs(){}


<<__EntryPoint>>
function main_reflection_function_to_string_003() {
$rfunc = new ReflectionFunction('zeroArgs');
echo $rfunc;
}
