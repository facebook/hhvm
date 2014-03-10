<?php

function main() {
  // Make a reasonable-sized test string
  $input = 'herp derp';
  for ($i = 0; $i <= 1024; ++$i) {
    $input .= crc32($input);
  }
  $fnam = tempnam('/tmp', 'test');
  $f = fopen($fnam, 'w');
  fwrite($f, gzdeflate($input));
  fclose($f);

  $compressed = file_get_contents($fnam);
  $f = fopen($fnam, 'r');
  stream_filter_append($f, 'zlib.inflate');
  $output = '';
  while (!feof($f)) {
    $output .= fread($f, 512);
  }
  var_dump(array(
    'input len' => strlen($input),
    'input md5' => md5($input),
    'output len' => strlen($output),
    'output md5' => md5($output)
  ));
  unlink($fnam);
}

main();
