<?php


<<__EntryPoint>>
function main_key() {
$array = array(
  "fruit1" => "apple",
  "fruit2" => "orange",
  "fruit3" => "grape",
  "fruit4" => "apple",
  "fruit5" => "apple"
);

// this cycle echoes all associative array
// key where value equals "apple"
$output = '';
while (true) {
  $fruit_name = current(&$array);
  if ($fruit_name === false) break;
  if ($fruit_name === 'apple') {
    $output .= key(&$array);
  }
  next(&$array);
}
var_dump($output);
$array = array();
var_dump(key(&$array));
}
