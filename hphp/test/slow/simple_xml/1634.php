<?hh


<<__EntryPoint>>
function main_1634() {
$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump((array)$node->bar);
}
