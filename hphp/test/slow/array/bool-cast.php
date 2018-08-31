<?php


function f($a) {
  var_dump((bool)$a);
}


<<__EntryPoint>>
function main_bool_cast() {
f($GLOBALS);
f(array('a' => 'b'));
f(array('a'));
f(array());
}
