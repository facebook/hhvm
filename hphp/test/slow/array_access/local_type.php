<?php

class thing {
  private $prop;

  function go() {
    $idx = 'five';
    $instances[$this->prop] = false;
    return isset($this->prop[$idx]);
  }
}

$t = new thing;
var_dump($t->go());
