<?php
$c = new TestClass;
$c->testwithctx();
testnoctx();

function testnoctx() {
  echo "testing from anonymous context\n";
  echo "testvar set before Req? " . isset($testvar) . "\n";
  require 'reqtests/mod.inc';
  echo "testvar set after Req? " . isset($testvar) . "\n";
}

class TestClass {
  private $var = 'hello';

  public function testwithctx() {
    echo "value of var before ReqOnce: " . $this->var . "\n";
    require_once 'reqtests/mod.inc';
    echo "value of var after ReqOnce: " . $this->var . "\n";
    echo "testvar set after ReqOnce? " . isset($testvar) . "\n";
  }
}
