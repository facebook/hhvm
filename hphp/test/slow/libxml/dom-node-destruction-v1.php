<?php
// c.f. http://3v4l.org/T1JaE

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
  $dom->appendChild($child);
  $dom->appendChild($dom->createElement('bar')->init());

  echo "Leave foo()\n";
  return [$dom, $child];
}

function bar() {
  echo "Enter bar()\n";
  list($d, $c) = foo();

  $fooList = $d->getElementsByTagName('foo');
  foreach ($fooList as $foo) {
    $foo->info();
  }
  echo "Leave bar()\n";
}

echo "Enter main()\n";
bar();
echo "Leave main()\n";
