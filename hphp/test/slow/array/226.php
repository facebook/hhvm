<?php


<<__EntryPoint>>
function main_226() {
$a = array('a' => 1, 'b' => 2);
foreach ($a as $b => &$c) {
  var_dump($b);
  unset($a['b']);
}
}
