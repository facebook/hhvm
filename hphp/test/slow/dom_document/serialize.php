<?hh


<<__EntryPoint>>
function main_serialize() {
$element = new DOMElement("element", "somevalue");
$result = serialize($element);
print $result . "\n";
}
