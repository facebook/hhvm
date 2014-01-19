<?php

function less($a, $b) {
 return $a < $b;
 }

function main($a) {
  usort($a, 'less');
  var_dump($a);
}

main(array(1,2));
