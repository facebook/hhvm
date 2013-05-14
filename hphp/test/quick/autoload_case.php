<?php
function __autoload($name) {
  var_dump($name);
  class TestA { public static $D; }
}
TestA::$D = 1;
