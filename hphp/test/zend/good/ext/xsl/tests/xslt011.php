<?hh
  class foo {
       function foo() {}
       function __toString() { return "not a DomNode object";}
  }
<<__EntryPoint>> function main(): void {
print "Test 11: php:function Support\n";
$dom = new DOMDocument();
  $dom->load(dirname(__FILE__)."/xslt011.xsl");
  $proc = new XSLTProcessor;
  $xsl = $proc->importStylesheet($dom);

  $xml = new DOMDocument();
  $xml->load(dirname(__FILE__)."/xslt011.xml");
  $proc->registerPHPFunctions();
  print $proc->transformToXML($xml);
}

  function foobar($id, $secondArg = "" ) {
    if (is_array($id)) {
        return $id[0]->value . " - " . $secondArg;
    } else {
        return $id . " - " . $secondArg;
    }
  }
  function nodeSet($id = null) {
      if ($id && is_array($id)) {
          return $id[0];
      } else {
          $dom = new DOMDocument;
          $dom->loadXML("<root>this is from an external DomDocument</root>");
          return $dom->documentElement;
      }
  }
  function nonDomNode() {
    return  new foo();
  }

  class aClass {
    static function aStaticFunction($id) {
        return $id;
    }
  }
