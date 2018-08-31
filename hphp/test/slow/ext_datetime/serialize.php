<?php

function check($name) {
  $o = new $name('now', new DateTimeZone('UTC'));
  $s = serialize($o);
  $o2 = unserialize($s);
  var_dump($o2 == $o);
  var_dump(isset($o2->_date_time) === false);
}

class A extends DateTime {}

<<__EntryPoint>>
function main_serialize() {
check('DateTime');
check('A');
}
