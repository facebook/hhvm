<?hh


<<__EntryPoint>>
function main_1634() :mixed{
$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump($node->bar);
}
