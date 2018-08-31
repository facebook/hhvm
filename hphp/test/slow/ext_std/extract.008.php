<?php

class A {
  public function __construct() {
    extract(['this' => 'a']);
    var_dump($this);
    unset($this);
    extract(['this' => 'a']);
    var_dump($this);
    extract(['this' => 'b']);
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_008() {
new A();
extract(['this' => 'a']);
var_dump($this);
extract(['this' => 'b']);
var_dump($this);
}
