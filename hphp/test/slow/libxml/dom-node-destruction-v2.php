<?hh
// c.f. http://3v4l.org/FtFt6

class MyElement extends DOMElement {
  public $id;
  public static $next = 0;

  function init() :mixed{
    $this->id = MyElement::$next++;
    echo "Initializing DOMElement #" . $this->id . ": "  . $this->tagName . "\n";

    return $this;
  }

  function info() :mixed{
    echo "Querying DOMElement #" . (string)($this->id) . ": "  . (string)($this->tagName) . "\n";
  }
}

function foo() :mixed{
  echo "Enter foo()\n";
  $dom = new DOMDocument;
  $dom->registerNodeClass('DOMElement', 'MyElement');

  $child = $dom->createElement('foo')->init();
  $grandchild = $dom->createElement('bar')->init();
  $greatgrandchild = $dom->createElement('fiz')->init();

  $grandchild->appendChild($greatgrandchild);
  $child->appendChild($grandchild);
  $dom->appendChild($child);

  echo "Leave foo()\n";
  return $grandchild;
}

function bar() :mixed{
  echo "Enter bar()\n";
  $g = foo();

  $fooList = $g->getElementsByTagName('fiz');
  foreach ($fooList as $foo) {
    $foo->info();
    var_dump($foo->getNodePath());
  }
  echo "Leave bar()\n";
}


<<__EntryPoint>>
function main_dom_node_destruction_v2() :mixed{
echo "Enter main()\n";
bar();
echo "Leave main()\n";
}
