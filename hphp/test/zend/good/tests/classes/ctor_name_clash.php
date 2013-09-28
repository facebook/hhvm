<?php
class base {
  function base() {
    echo __CLASS__."::".__FUNCTION__."\n";
  }
}

class derived extends base {
  function base() {
    echo __CLASS__."::".__FUNCTION__."\n";
  }
}

$obj = new derived();
$obj->base();
?>