<?php

function main() {
  $x = array();
  $x[4] = 43;
  print $x[4] . "\n";

  $i = 131664776877370;
  var_dump(isset($x[131664776877370]));
  $x[$i] = 'foo';
  var_dump(isset($x[131664776877370]));
}
main();
