<?php

function main() {
  $fname = tempnam('/tmp', 'foo');

  $in = 'herpderp';
  // Build a large blob that's hard to compress
  for ($i = 0; $i < 1000; ++$i) {
    $in .= md5($in);
  }
  var_dump('input md5: '.md5($in));

  $f = fopen($fname, 'r+');
  $filter = stream_filter_append($f, 'zlib.deflate', STREAM_FILTER_WRITE);
  foreach (str_split($in, 8096) as $chunk) {
    fwrite($f, $chunk);
  }
  stream_filter_remove($filter);
  var_dump('deflated size: '.fstat($f)['size']);
  fclose($f);

  $contents = file_get_contents($fname);
  var_dump('deflated contents: '.md5($contents));

  $f = fopen($fname, 'r+');
  $filter = stream_filter_append($f, 'zlib.inflate', STREAM_FILTER_WRITE);
  foreach (str_split($contents, 8096) as $chunk) {
    var_dump('chunk write: '.strlen($chunk));
    fwrite($f, $chunk);
  }
  stream_filter_remove($filter);
  var_dump('stream size: '.fstat($f)['size']);
  rewind($f);

  var_dump('inflated stream size: '.fstat($f)['size']);

  var_dump('inflated stream md5: '.md5(fread($f, fstat($f)['size'])));
  fclose($f);
  var_dump('inflated file size: '.stat($fname)['size']);
  var_dump('inflated file md5: '.md5(file_get_contents($fname)));
  unlink($fname);

/* Handy for debugging - should match. Commented out as it's HHVM-only.

  $inflator = new __SystemLib_ChunkedInflator();
  $output = '';
  foreach (str_split($contents, 8096) as $chunk) {
    $output .= $inflator->inflateChunk($chunk);
  }
  var_dump('chunkedinflator size: '.strlen($output));
  var_dump('chunkedinflator md5: '.md5($output));
*/
}

main();
