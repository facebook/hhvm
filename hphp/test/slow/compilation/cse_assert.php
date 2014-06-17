<?php

function test($x) {
  $y = ($x ? 5 : 3) + 5;
  return array($y + 1, $y + 1,);
}

var_dump(test(0));
