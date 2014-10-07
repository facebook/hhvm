<?php
$change = array('GLOBALS' => 'test');
var_dump(extract($change, EXTR_OVERWRITE));
var_dump($GLOBALS);

class Test {
  public function testOverwrite($array) {
    var_dump(extract($array, EXTR_OVERWRITE));
    var_dump($this);
  }
}

$t = new Test();
$t->testOverwrite(array('this' => 'foobar'));
