<?hh

function zeroArgs():mixed{}


<<__EntryPoint>>
function main_reflection_function_to_string_003() :mixed{
$rfunc = new ReflectionFunction('zeroArgs');
echo $rfunc;
}
