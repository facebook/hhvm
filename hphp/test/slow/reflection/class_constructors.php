<?php
class NewStyle {
  public function __construct() {
  }
}

class SubNewStyle extends NewStyle {
}

class OldStyle {
  public function OldStyle($x) {
  }
}

class SubOldStyle extends OldStyle {
}


<<__EntryPoint>>
function main_class_constructors() {
var_dump((new ReflectionClass('NewStyle'))->getConstructor()->getName());
var_dump((new ReflectionClass('SubNewStyle'))->getConstructor()->getName());

var_dump((new ReflectionClass('OldStyle'))->getConstructor()->getName());
var_dump((new ReflectionClass('SubOldStyle'))->getConstructor()->getName());
}
