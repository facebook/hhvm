<?php

function f(&$a) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1077() {
  $a = array('b' => &$c);
  f(&$c);
  var_dump($a);
}
