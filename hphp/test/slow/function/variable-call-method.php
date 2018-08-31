<?php

function g() {
  $f = 'IntlChar::ord';
  var_dump($f(' '));

  $f = array('IntlChar', 'ord');
  var_dump($f('='));

  $o = new UConverter('utf-8', 'latin1');
  $f = array($o, 'convert');
  var_dump($f('foo'));
}


<<__EntryPoint>>
function main_variable_call_method() {
g();
}
