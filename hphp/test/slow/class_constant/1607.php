<?php
function foo($a = FOO) {
  echo $a;
}


<<__EntryPoint>>
function main_1607() {
define('FOO', 3);
foo();
}
