<?hh


<<__EntryPoint>>
function main_serialize() :mixed{
$element = new DOMElement("element", "somevalue");
$result = serialize($element);
print $result . "\n";
}
