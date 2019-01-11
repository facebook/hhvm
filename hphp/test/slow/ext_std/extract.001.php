<?php

class Test {
  public function testOverwrite($array) {
    var_dump(extract(&$array, EXTR_OVERWRITE));
    var_dump($this);
  }
}

<<__EntryPoint>>
function main_extract_001() {
$change = array('GLOBALS' => 'test');
var_dump(extract(&$change, EXTR_OVERWRITE));
var_dump($GLOBALS);

$t = new Test();
$t->testOverwrite(array('this' => 'foobar'));
}
