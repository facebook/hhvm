<?hh


<<__EntryPoint>>
function main_1636() {
$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump((string)$node->offsetGet(0));
}
