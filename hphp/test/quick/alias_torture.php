<?php

<<__EntryPoint>>
function foo() {

  $GLOBALS['a'] = 123;
  $b = $GLOBALS;
  $b['a'] = "of"; // via SetM
  var_dump($GLOBALS['a']);
}
