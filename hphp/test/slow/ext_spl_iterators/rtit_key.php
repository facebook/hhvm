<?php

$rait = new RecursiveArrayIterator([
  'a' => 0,
  1,
  'b' => array(
    2,
    3,
    'c'=>array(
      'd'=>4,
      5
    ),
    6,
    7
  ),
  8,
  9,
  array(
    0,
    1
  )
]
);

$rtit = new RecursiveTreeIterator($rait);
$rtit_bypass = new RecursiveTreeIterator($rait, RecursiveTreeIterator::BYPASS_CURRENT);


foreach($rtit as $key=>$val) {
  var_dump($rtit->key());
}

foreach($rtit_bypass as $key=>$val) {
  var_dump($rtit_bypass->key());
}
