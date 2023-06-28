<?hh

class MyNode extends DOMNode {
}
class MyElement extends DOMElement {
}

<<__EntryPoint>>
function main_1681() :mixed{
$dom = new DOMDocument;
var_dump($dom->registerNodeClass('DOMNode', 'MyNode'));
var_dump($dom->registerNodeClass('DOMElement', 'MyElement'));
}
