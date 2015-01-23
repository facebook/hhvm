<?php

// -----------------------------------------------------------
// 1. User-level doesn't override implicit native magic props.
// -----------------------------------------------------------

class MyTextNode extends DOMText {
  public function __get($name) {
    return "__get: $name";
  }
}
$dom = new DOMDocument();
$dom->registerNodeClass('DOMText', 'MyTextNode');
$node = $dom->appendChild($dom->createElement('Foo', 'Bar'));

var_dump($node->firstChild->textContent); // Impl-level
var_dump($node->firstChild->nonExisting); // User-level __get

// -----------------------------------------------------------
// 2. Explicit override of the native magic prop.
// -----------------------------------------------------------

class MyTextExplicit extends DOMText {
  private $textContent;
  public function __construct() {
    unset($this->textContent);
  }

  public function __get($name) {
    return "__get: $name";
  }
}

$my_text = new MyTextExplicit('Foo', 'Bar');

var_dump($my_text->textContent); // User-level
var_dump($my_text->nonExisting); // User-level

// -----------------------------------------------------------
// 3. Explicit override of simple prop.
// -----------------------------------------------------------

class MyTextDirect extends DOMText {
  public $textContent;
  public function __construct() {
    $this->textContent = 'foo';
  }
}

$my_text = new MyTextDirect('Foo', 'Bar');

var_dump($my_text->textContent); // 10
var_dump($my_text->nonExisting); // Unhandled
