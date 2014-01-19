<?php

function val() { return 0; }
function foo($k) {
  $array = array(0, 1);
  $idx =& val();
  for ($ik = 0; $ik < 10; ++$ik) {
  }

  $idx++;
  echo $array[$idx];
  echo "\n";
}

foo(12);
