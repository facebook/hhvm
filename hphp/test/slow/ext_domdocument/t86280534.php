<?hh

function test1() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->isSameNode(new stdClass);
}

function test2() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->importNode(new stdClass);
}

function test3() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->saveHTML(new stdClass);
}

function test4() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->saveXML(new stdClass);
}

function test5() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->removeAttributeNode(new stdClass);
}

function test6() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setAttributeNode(new stdClass);
}

function test7() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setAttributeNodeNS(new stdClass);
}

function test8() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setIDAttributeNode(new stdClass, false);
}

function test9() {
  $impl= new DOMImplementation;
  $impl->createDocument("abc", "abc", new stdClass);
}

function test10() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->insertBefore(new stdClass);
}

function test11() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->insertBefore($doc->createElement("node"), new stdClass);
}

function test12() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->removeChild(new stdClass);
}

function test13() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->replaceChild(new stdClass, $doc->createElement("node"));
}

function test14() {
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->replaceChild($node, new stdClass);
}

function test15() {
  $doc = new DOMDocument();
  $xpath = new DOMXpath($doc);
  $xpath->evaluate("abc", new stdClass);
}

function test16() {
  $doc = new DOMDocument();
  $xpath = new DOMXpath($doc);
  $xpath->query("abc", new stdClass);
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
