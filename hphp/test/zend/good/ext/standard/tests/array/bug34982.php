<?php
$ar = array(
    'element 1',
    array('subelement1')
    );

func($ar);
print_r($ar);

function func($a) {
  array_walk_recursive($a, 'apply');
  print_r($a);
}

function apply(&$input, $key) {
  $input = 'changed';
}
?>