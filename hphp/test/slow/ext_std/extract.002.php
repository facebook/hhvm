<?php
var_dump(extract(
  array(
      'GLOBALS' => 'test',
    ),
  EXTR_OVERWRITE
));
var_dump($GLOBALS);

class Test {
  public function testOverwrite() {
      var_dump(extract(array('this' => 'foobase'), EXTR_OVERWRITE));
      var_dump($this);
    }
}

$t = new Test();
$t->testOverwrite();
