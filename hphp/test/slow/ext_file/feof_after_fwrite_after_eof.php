<?php

function main() {
  $inputs = array(
    'php://temp',
    'php://memory',
  );

  foreach ($inputs as $input) {
    printf("---%s---\n", $input);
    $f = fopen($input, 'r+');
    fread($f, 1);
    var_dump(feof($f));
    fwrite($f, 'foo');
    var_dump(feof($f));
    fseek($f, 0, SEEK_CUR);
    var_dump(feof($f));
  }
}

main();
