<?php

class A {
  function good($arg) {
    global $method;
    $method = 'bad';
    var_dump('good');
  }
  function bad($arg) {
    var_dump('bad');
  }
}

$method = 'good';
preg_replace_callback('/foo/', array('A', $method), 'foo foo');
$method = 'good';
preg_replace_callback('/foo/', array('A', &$method), 'foo foo');
