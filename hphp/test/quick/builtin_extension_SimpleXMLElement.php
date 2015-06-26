<?php

include __DIR__."/builtin_extensions.inc";

class A_SimpleXMLElement extends SimpleXMLElement {
  public $___x;
}
test("SimpleXMLElement", "<?xml version='1.0'?><document></document>");
