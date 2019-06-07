<?hh


<<__EntryPoint>>
function main_1638() {
$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump((string)$node);
}
