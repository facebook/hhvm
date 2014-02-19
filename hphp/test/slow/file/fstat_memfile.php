<?php

function main() {
  // php://input is backed by a MemFile
  $f = fopen('php://input', 'r');
  var_dump(fstat($f));
}

main();
