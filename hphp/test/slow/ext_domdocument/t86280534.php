<?hh

function test1() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->isSameNode(new stdClass);
}

function test2() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->importNode(new stdClass);
}

function test3() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->saveHTML(new stdClass);
}

function test4() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $doc->saveXML(new stdClass);
}

function test5() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->removeAttributeNode(new stdClass);
}

function test6() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setAttributeNode(new stdClass);
}

function test7() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setAttributeNodeNS(new stdClass);
}

function test8() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->setIDAttributeNode(new stdClass, false);
}

function test9() :mixed{
  $impl= new DOMImplementation;
  $impl->createDocument("abc", "abc", new stdClass);
}

function test10() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->insertBefore(new stdClass);
}

function test11() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->insertBefore($doc->createElement("node"), new stdClass);
}

function test12() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->removeChild(new stdClass);
}

function test13() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->replaceChild(new stdClass, $doc->createElement("node"));
}

function test14() :mixed{
  $doc = new DOMDocument();
  $node = $doc->createElement("node");
  $node->replaceChild($node, new stdClass);
}

function test15() :mixed{
  $doc = new DOMDocument();
  $xpath = new DOMXPath($doc);
  $xpath->evaluate("abc", new stdClass);
}

function test16() :mixed{
  $doc = new DOMDocument();
  $xpath = new DOMXPath($doc);
  $xpath->query("abc", new stdClass);
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $tests = vec[
    test1<>,
    test2<>,
    test3<>,
    test4<>,
    test5<>,
    test6<>,
    test7<>,
    test8<>,
    test9<>,
    test10<>,
    test11<>,
    test12<>,
    test13<>,
    test14<>,
    test15<>,
    test16<>
  ];

  foreach ($tests as $t) {
    try {
      $t();
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
