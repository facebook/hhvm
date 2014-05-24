<?php

class A {
  public function __init__() {
    var_dump('shoulnd\'t be called');
  }
}
new A;
var_dump((new Exception)->__init__());
