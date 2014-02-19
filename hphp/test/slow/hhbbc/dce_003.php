<?php

function heh($ar) {
  if (!$ar) throw new Exception('a');
  return 42;
}
function bar($ar) {
  $tmp = 54;
  try {
    $tmp = heh($ar);
  } catch (Exception $x) {
    var_dump($tmp);
  }
  var_dump($tmp);
}
bar(array('a'));
bar(array());

