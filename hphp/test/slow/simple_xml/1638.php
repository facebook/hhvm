<?hh


<<__EntryPoint>>
function main_1638() :mixed{
$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump((string)$node);
}
