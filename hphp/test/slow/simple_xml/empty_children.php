<?hh


<<__EntryPoint>>
function main_empty_children() :mixed{
$x = new SimpleXMLElement("<foo><x /></foo>");
var_dump($x->x->children()->offsetGet(0));
}
