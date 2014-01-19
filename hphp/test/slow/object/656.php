<?php

class EE extends Exception {
}
class E extends EE {
  function foo() {
}
  function __construct() {
    echo 'MAKING E';
    parent::__construct();
  }
}
new E;
