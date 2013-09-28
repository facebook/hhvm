<?php

function f($x) {
  var_dump(is_array($x), $x[0]);
}
f(array(0));
f('foo');
