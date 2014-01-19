<?php

function main() {
  $asd = array("asd", array("bar","baz"));
  $bsd = "asd";
  echo ($$bsd)[1][1] . "\n";

  $bsd = "unknown";
  echo ($$bsd)[1][1] . "\n";
}
main();
