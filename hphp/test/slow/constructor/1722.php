<?php

if (isset($g)) {
  class X {
}
}
 else {
  class X {
function X() {
var_dump(__METHOD__);
}
}
}
class Y extends X {
  function __construct($a, $b) {
    var_dump(__METHOD__);
    parent::__construct($a,$b);
  }
}
$y = new Y(1,2);
