<?php

class MyDoc extends DOMDocument {
  function __construct($version, $encoding) {}
}

$doc = new MyDoc(1, "");
$doc->normalizeDocument(); // warning

class MyOtherDoc extends DOMDocument {
  function __construct($version, $encoding) {
    parent::__construct($version, $encoding);
  }
}

$doc = new MyOtherDoc(1, "");
$doc->normalizeDocument();
echo "ok\n";
