<?hh


<<__EntryPoint>>
function main_1639() :mixed{
$node = new SimpleXMLElement('<foo>whoops</foo>');
var_dump($node->__toString());
}
