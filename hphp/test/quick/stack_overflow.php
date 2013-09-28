<?php

$g = array(1,2,3);
function cmp($a, $b) {
  global $g;
  usort($g, 'cmp');
  fiz();
}

cmp(0, 0);

function fiz() {
  var_dump(1);
}
