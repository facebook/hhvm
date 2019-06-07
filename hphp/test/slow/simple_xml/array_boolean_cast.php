<?hh


<<__EntryPoint>>
function main_array_boolean_cast() {
$config = simplexml_load_string('<config><prepare /></config>');
$configArray = (array)$config;

var_dump((bool)$config->prepare);
var_dump((bool)$configArray['prepare']);
}
