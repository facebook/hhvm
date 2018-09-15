<?php

class MyWrapper {
  private $ctorRan = false;
  public function __construct() {
    $this->ctorRan = true;
  }
  public function stream_open($fn, $mode, $opt, &$opened_path) {
    var_dump($this->ctorRan);
    return true;
  }
}

<<__EntryPoint>>
function main_1695() {
stream_wrapper_register('wrap', 'MyWrapper');
fclose(fopen('wrap://test', 'r'));
}
