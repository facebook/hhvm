<?php

class A {
  public function __construct() {
    $arr = ['this' => 'a'];
    var_dump(extract(&$arr));
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_007() {
new A();
}
