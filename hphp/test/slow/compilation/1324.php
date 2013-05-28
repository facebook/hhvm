<?php

function foo(&$a, &$b) {
}
if (isset($g)) {
  function foo($a, $b) {
}
}
function bar() {
  foo($x, $y);
}
bar();
