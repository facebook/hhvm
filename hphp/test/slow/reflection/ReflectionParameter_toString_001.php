<?hh

function ref(inout $a) {}


<<__EntryPoint>>
function main_reflection_parameter_to_string_001() {
$rfunc = new ReflectionFunction('ref');
$rparam = $rfunc->getParameters();
echo $rparam[0];
}
