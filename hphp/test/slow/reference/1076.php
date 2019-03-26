<?php

<<__EntryPoint>>
function main_1076() {
  $a = array('b' => &$c);
  $c = 'ok';
  var_dump($a);
}
