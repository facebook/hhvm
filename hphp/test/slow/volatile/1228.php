<?php

function __autoload($name) {
  if ($name == 'CaT') {
  class CaT {
    function __construct() {
}
  }
  }
  var_dump($name);
}
new CaT(1);
class_exists('cat', false);
