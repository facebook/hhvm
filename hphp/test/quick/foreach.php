<?php
$x = array('1',2,'3');
foreach ($x as $k => $v) {
  echo $k;
  echo ' ';
  echo $v;
  echo "\n";
}


// Test case pretty much stolen from www
function array_glue($pre, $array, $post) {
  foreach ($array as &$v) {
    $v = $pre.$v.$post;
  }

  return $array;
}


function goo($arr) {
  return array_glue('fub', $arr, '');
}


$array = array(1,2,3);
$array []= 400;  // make it non-static

$new = goo($array);
var_dump($array);
var_dump($new);
