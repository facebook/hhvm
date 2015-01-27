<?php

class A {
  public function __construct() {
    var_dump(extract(['this' => 'a']));
    var_dump($this);
  }
}

new A();
