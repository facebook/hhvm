<?php
class NewStyle {
  public function __construct() {
  }
}

class SubNewStyle extends NewStyle {
}

var_dump((new ReflectionClass('NewStyle'))->getConstructor()->getName());
var_dump((new ReflectionClass('SubNewStyle'))->getConstructor()->getName());

class OldStyle {
  public function OldStyle($x) {
  }
}

class SubOldStyle extends OldStyle {
}

var_dump((new ReflectionClass('OldStyle'))->getConstructor()->getName());
var_dump((new ReflectionClass('SubOldStyle'))->getConstructor()->getName());
