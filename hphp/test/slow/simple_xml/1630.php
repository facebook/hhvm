<?hh


<<__EntryPoint>>
function main_1630() :mixed{
$x = new SimpleXMLElement('<foo><bar>345.234</bar></foo>');
var_dump((float)$x->bar);
}
