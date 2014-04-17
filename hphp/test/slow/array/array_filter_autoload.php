<?php
function __autoload($c) {
  throw new Exception("Ha!");
}
function test() {
  try {
    array_filter(array(1), array('Foo', 'Bar'));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

test();
