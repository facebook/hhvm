<?php

class Test {
  public function testOverwrite() {
    $arr = array('this' => 'foobase');
    var_dump(extract(&$arr, EXTR_OVERWRITE));
    var_dump($this);
  }
}

<<__EntryPoint>>
function main_extract_002() {
  $arr = array('GLOBALS' => 'test');
  var_dump(extract(&$arr, EXTR_OVERWRITE));
  var_dump($GLOBALS);

  $t = new Test();
  $t->testOverwrite();
}
