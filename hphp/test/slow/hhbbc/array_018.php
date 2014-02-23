<?php

function four() { return 4; }
function arr() { return array('x' => four()); }
function go() {
  $x = arr();
  $x['something']->hahaha = "yeah";
  return $x;
}
function main() {
  $x = go();
  var_dump($x['something']);
  var_dump(is_object($x['something']));
}
main();
