<?php

class A {
  public function __construct() {
    var_dump(extract(['this' => 'a']));
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_007() {
new A();
}
