<?hh


<<__EntryPoint>>
function main_boolean_cast() :mixed{
$element = new SimpleXMLElement('<root />');
var_dump((bool)$element);

$element = new SimpleXMLElement('<root><child /></root>');
var_dump((bool)$element->child);
}
