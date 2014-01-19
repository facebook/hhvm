<?php

class Test {
  public function __call($name, $args) {
    var_dump($args);
  }
}
$test = new Test();
$test->test();
