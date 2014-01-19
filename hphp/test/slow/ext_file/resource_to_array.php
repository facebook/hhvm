<?php

function main() {
  $tempfile = tempnam('/tmp', 'vmextfiletest');
  $f = fopen($tempfile, 'w');
  var_dump((array)$f);
  unlink($tempfile);
}

main();
