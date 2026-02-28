<?hh
// c.f. http://3v4l.org/T1JaE

class MyElement extends DOMElement {
  public $id;
  public static $next = 0;

  function init() :mixed{
    $this->id = MyElement::$next++;
    echo "Initializing DOMElement #" . $this->id . ": "  . $this->tagName . "\n";

    return $this;
  }

  function info() :mixed{
    echo "Querying DOMElement #" . $this->id . ": "  . $this->tagName . "\n";
  }
}

function foo() :mixed{
  echo "Enter foo()\n";
  $dom = new DOMDocument;
  $dom->registerNodeClass('DOMElement', 'MyElement');

  $child = $dom->createElement('foo')->init();
  $dom->appendChild($child);
  $dom->appendChild($dom->createElement('bar')->init());

  echo "Leave foo()\n";
  return vec[$dom, $child];
}

function bar() :mixed{
  echo "Enter bar()\n";
  list($d, $c) = foo();

  $fooList = $d->getElementsByTagName('foo');
  foreach ($fooList as $foo) {
    $foo->info();
  }
  echo "Leave bar()\n";
  __hhvm_intrinsics\launder_value($c);
}


<<__EntryPoint>>
function main_dom_node_destruction_v1() :mixed{
echo "Enter main()\n";
bar();
echo "Leave main()\n";
}
