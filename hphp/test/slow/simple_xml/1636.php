<?hh


<<__EntryPoint>>
function main_1636() :mixed{
$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump((string)$node->offsetGet(0));
}
