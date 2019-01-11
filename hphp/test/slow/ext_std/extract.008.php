<?php

class A {
  public function __construct() {
    $arr = ['this' => 'a'];
    extract(&$arr);
    var_dump($this);
    unset($this);
    $arr = ['this' => 'a'];
    extract(&$arr);
    var_dump($this);
    $arr = ['this' => 'b'];
    extract(&$arr);
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_008() {
  new A();
  $arr = ['this' => 'a'];
  extract(&$arr);
  var_dump($this);
  $arr = ['this' => 'b'];
  extract(&$arr);
  var_dump($this);
}
