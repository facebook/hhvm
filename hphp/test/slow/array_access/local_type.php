<?php

class thing {
  private $prop;

  function go() {
    $idx = 'five';
    $instances = array();
    $instances[$this->prop] = false;
    return isset($this->prop[$idx]);
  }
}


<<__EntryPoint>>
function main_local_type() {
$t = new thing;
var_dump($t->go());
}
