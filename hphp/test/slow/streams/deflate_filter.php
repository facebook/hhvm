<?php

function main() {
  $fnam = tempnam('/tmp', 'test');
  $f = fopen($fnam, 'w');
  stream_filter_append($f, 'zlib.deflate', STREAM_FILTER_WRITE);
  fwrite($f, 'Hello, world.');
  fclose($f);
  var_dump(md5(file_get_contents($fnam)));
  unlink($fnam);
}

main();
