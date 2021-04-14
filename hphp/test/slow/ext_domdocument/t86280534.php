<?hh

function test1() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->isSameNode(new stdclass);
}

function test2() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->importNode(new stdclass);
}

function test3() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->saveHTML(new stdclass);
}

function test4() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->saveXML(new stdclass);
}

function test5() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->removeAttributeNode(new stdclass);
}

function test6() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setAttributeNode(new stdclass);
}

function test7() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setAttributeNodeNS(new stdclass);
}

function test8() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setIDAttributeNode(new stdclass, false);
}

function test9() {
  $impl= new DOMImplementation;
  $impl->createDocument("abc", "abc", new stdclass);
}

function test10() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->insertBefore(new stdclass);
}

function test11() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->insertBefore($doc->createElement("node"), new stdclass);
}

function test12() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->removeChild(new stdclass);
}

function test13() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->replaceChild(new stdclass, $doc->createElement("node"));
}

function test14() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->replaceChild($node, new stdclass);
}

function test15() {
  $doc = new DOMDocument();
  $xpath = new DOMXpath($doc);
  $xpath->evaluate("abc", new stdclass);
}

function test16() {
  $doc = new DOMDocument();
  $xpath = new DOMXpath($doc);
  $xpath->query("abc", new stdclass);
}

<<__EntryPoint>>
function main() {
  $tests = vec[
    fun('test1'),
    fun('test2'),
    fun('test3'),
    fun('test4'),
    fun('test5'),
    fun('test6'),
    fun('test7'),
    fun('test8'),
    fun('test9'),
    fun('test10'),
    fun('test11'),
    fun('test12'),
    fun('test13'),
    fun('test14'),
    fun('test15'),
    fun('test16')
  ];

  foreach ($tests as $t) {
    try {
      $t();
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
