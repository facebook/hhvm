<?php

class MyNode extends DOMNode {
}
class MyElement extends DOMElement {
}
$dom = new DOMDocument;
var_dump($dom->registerNodeClass('DOMNode', 'MyNode'));
var_dump($dom->registerNodeClass('DOMElement', 'MyElement'));
