<?php

function main() {
  $a = hphp_msarray();
  $a['foo'] = 2;
  $a['bar'] = 1;

  asort($a);
  var_dump($a);
}

main();
