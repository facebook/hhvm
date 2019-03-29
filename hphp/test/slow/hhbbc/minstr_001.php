<?php

function foo() {
  $lol = new stdclass;
  $x = [$lol => 2];
  var_dump($x);
}



<<__EntryPoint>>
function main_minstr_001() {
foo();
}
