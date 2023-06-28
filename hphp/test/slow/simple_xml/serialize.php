<?hh


<<__EntryPoint>>
function main_serialize() :mixed{
$element = new \SimpleXMLElement("<foo><bar>baz</bar></foo>");
var_dump(json_encode($element));
}
