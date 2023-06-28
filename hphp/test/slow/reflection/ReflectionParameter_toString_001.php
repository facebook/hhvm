<?hh

function ref(inout $a) :mixed{}


<<__EntryPoint>>
function main_reflection_parameter_to_string_001() :mixed{
$rfunc = new ReflectionFunction('ref');
$rparam = $rfunc->getParameters();
echo $rparam[0];
}
