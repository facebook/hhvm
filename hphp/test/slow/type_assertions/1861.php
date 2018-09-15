<?php

function f($x) {
  var_dump(is_array($x), $x[0]);
}

<<__EntryPoint>>
function main_1861() {
f(array(0));
f('foo');
}
