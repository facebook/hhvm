<?php

function foo(&$a, &$b) {
}
function bar() {
  foo(&$x, &$y);
}

<<__EntryPoint>>
function main_1324() {
if (isset($g)) {
  function foo(&$a, &$b) {
}
}
bar();
}
