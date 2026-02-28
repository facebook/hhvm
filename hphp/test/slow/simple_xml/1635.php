<?hh


<<__EntryPoint>>
function main_1635() :mixed{
$node = new SimpleXMLElement('<foo><bar name="value">whoops</bar></foo>');
var_dump($node->bar);
}
