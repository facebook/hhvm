<?hh

class SampleElement extends DOMElement {}

<<__EntryPoint>>
function main_create_element() {
$dom = new DOMDocument();
$dom->registerNodeClass('DOMElement', 'SampleElement');

var_dump(get_class($dom->createElement('test')));
var_dump(get_class($dom->createElementNS('n:s', 'test')));
}
