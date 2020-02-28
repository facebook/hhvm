<?hh

// -----------------------------------------------------------
// 1. User-level doesn't override implicit native magic props.
// -----------------------------------------------------------

class MyElement extends DOMElement {
  private $props = darray['userMagic' => 'userMagic'];
  public function __get($name) {
    echo "__get {$name}: ";
    if (array_key_exists($name, $this->props)) {
      return $this->props[$name];
    }
  }
}

// -----------------------------------------------------------
// 2. Explicit override of the native magic prop.
// -----------------------------------------------------------

class MyElementExplicit extends DOMElement {
  private $nodeValue;
  public function __construct() {
    unset($this->nodeValue);
  }

  public function __get($name) {
    echo "__get {$name}: ";
    return "$name-value";
  }
}

// -----------------------------------------------------------
// 3. Explicit override of simple prop.
// -----------------------------------------------------------

class MyElementDirect extends DOMElement {
  public $nodeValue = 10;
  public function __construct() {
    $this->nodeValue = 10;
  }
}


<<__EntryPoint>>
function main_native_magic_props_domelement() {
$dom = new DOMDocument();
$dom->registerNodeClass('DOMElement', 'MyElement');
$dom->appendChild($dom->createElement('Foo', 'Bar'));

var_dump($dom->documentElement->nodeValue); // Implementaiton-level
var_dump($dom->documentElement->userMagic); // User-level, handled
var_dump($dom->documentElement->nonExisting); // User-level, unhandled

$my_elem = new MyElementExplicit('Foo', 'Bar');

var_dump($my_elem->nodeValue); // User-level
var_dump($my_elem->nonExisting); // User-level

$my_elem = new MyElementDirect('Foo', 'Bar');

var_dump($my_elem->nodeValue); // 10
var_dump($my_elem->nonExisting); // Unhandled
}
