<?php

class Test {
  public function testOverwrite() {
      var_dump(extract(array('this' => 'foobase'), EXTR_OVERWRITE));
      var_dump($this);
    }
}

<<__EntryPoint>>
function main_extract_002() {
var_dump(extract(
  array(
      'GLOBALS' => 'test',
    ),
  EXTR_OVERWRITE
));
var_dump($GLOBALS);

$t = new Test();
$t->testOverwrite();
}
