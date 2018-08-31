<?php

function less($a, $b) {
 return $a < $b;
 }

function main($a) {
  usort(&$a, 'less');
  var_dump($a);
}


<<__EntryPoint>>
function main_usort() {
main(array(1,2));
}
