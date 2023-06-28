<?hh


<<__EntryPoint>>
function main_1631() :mixed{
$x = new SimpleXMLElement('<foo><bar></bar></foo>');
var_dump((bool)$x->bar);
}
