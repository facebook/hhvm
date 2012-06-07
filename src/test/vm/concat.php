<?php

function main() {
  $s = "a" . "b";
  print $s."\n";

  $s = "a" . array();
  print $s."\n";

  $s = array() . "b";
  print $s."\n";

  $s = array() . array();
  print $s."\n";
}

main();

