<?php

function foo() {
  $x = new stdClass;
  $x->v = array(1, 2);
  return $x;
}
foo()->v[0] += 5;
var_dump(shuffle(foo()->v));
