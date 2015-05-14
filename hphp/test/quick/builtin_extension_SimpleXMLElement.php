<?php

include __DIR__."/builtin_extensions.inc";

class A_SimpleXMLElement extends SimpleXMLElement {
  public $___x;
}
test("SimpleXMLElement");


function basicXML() {
  return
'<root>
  <a><a1>a1</a1></a>
  <b>b</b>
  <c>c1</c>
</root>';
}

function testIteration() {
  $s = new SimpleXMLElement(basicXMl());
  foreach ($s as $i => $e) {
    echo 'Of basic XML structure the ';
    echo ($i+1) . "th nodes contents is: $e\n";
  }
}
testIteration();
