<?hh


<<__EntryPoint>>
function main_savexml_exists() {
$node = simplexml_load_string('<root />');
var_dump(method_exists($node, 'saveXML'));
}
