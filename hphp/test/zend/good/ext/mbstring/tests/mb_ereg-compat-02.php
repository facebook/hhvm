<?php
/* (counterpart: ext/standard/tests/reg/005.phpt) */
  $a="This is a nice and simple string";
  echo mb_ereg(".*(is).*(is).*",$a,$registers);
  echo "\n";
  echo $registers[0];
  echo "\n";
  echo $registers[1];
  echo "\n";
  echo $registers[2];
  echo "\n";
?>