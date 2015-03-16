<?php
// c.f. http://3v4l.org/FtFt6

class MyElement extends DOMElement {
  public $id;
  public static $next = 0;

  function init() {
    $this->id = MyElement::$next++;
    echo "Initializing DOMElement #" . $this->id . ": "  . $this->tagName . "\n";

    return $this;
  }

  function __destruct() {
    echo "Destructing DOMElement #" . $this->id . ": "  . $this->tagName . "\n";
  }

  function info() {
    echo "Querying DOMElement #" . $this->id . ": "  . $this->tagName . "\n";
  }
}

function foo() {
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

function bar() {
  echo "Enter bar()\n";
  $g = foo();

  $fooList = $g->getElementsByTagName('fiz');
  foreach ($fooList as $foo) {
    $foo->info();
    var_dump($foo->getNodePath());
  }
  echo "Leave bar()\n";
}

echo "Enter main()\n";
bar();
echo "Leave main()\n";
