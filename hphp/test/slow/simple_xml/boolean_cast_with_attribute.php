<?hh


<<__EntryPoint>>
function main_boolean_cast_with_attribute() :mixed{
$request = "<?xml version=\"1.0\"?>\n<xml have=\"attributes\" />\n";
$xml = new SimpleXMLElement($request);
var_dump(!$xml);
}
