<?hh

function main() {
  $input = 'herp derp';
  // need a bigger string, with big output
  for ($i = 0; $i <= 1024; ++$i) {
    $input .= crc32($input);
  }
  $compressed = gzdeflate($input);
  // $input is ~ 10kb, $compressed is ~ 5kb
  $chunks = str_split($compressed, 512 /* bytes */);

  $output = '';
  $inflator = new __SystemLib_ChunkedInflator();
  $i = 0;
  foreach ($chunks as $chunk) {
    $this_chunk = $inflator->inflateChunk($chunk);
    printf(
      "Chunk %d: produced %d; eof: %s\n",
      $i++,
      strlen($this_chunk),
      $inflator->eof() ? 'true' : 'false'
    );
    $output .= $this_chunk;
  }

  printf(
    "-----\nInput size: %d\nOutput size: %d\nIn MD5: %s\nOut MD5: %s\n",
    strlen($input),
    strlen($output),
    md5($input),
    md5($output),
  );
}

main();
