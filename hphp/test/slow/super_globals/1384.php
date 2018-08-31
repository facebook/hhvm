<?php

function test() {
  unset($GLOBALS['_SERVER']);
  $GLOBALS['_SERVER']['foo'] = 'bar';
  var_dump($_SERVER['foo']);
  }

<<__EntryPoint>>
function main_1384() {
test();
}
