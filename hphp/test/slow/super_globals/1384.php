<?php

function test() {
  unset($GLOBALS['_SERVER']);
  $GLOBALS['_SERVER']['foo'] = 'bar';
  var_dump($_SERVER['foo']);
  }
test();
