<?hh

function test($input, $compressed, $decompressor) {
  $output = '';
  $i = 0;
  $chunks = str_split($compressed, 512 /* bytes */);
  foreach ($chunks as $chunk) {
    $this_chunk = $decompressor->inflateChunk($chunk);
    printf(
      "Chunk %d: produced %d; eof: %s\n",
      $i++,
      strlen($this_chunk),
      $decompressor->eof() ? 'true' : 'false'
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

function main() {
  $input = 'herp derp';
  // need a bigger string, with big output
  for ($i = 0; $i <= 1024; ++$i) {
    $input .= crc32($input);
  }
  printf("-----\nTesting inflator\n");
  test($input, gzdeflate($input), new __SystemLib\ChunkedInflator());

  printf("-----\nTesting gunzipper\n");
  test($input, gzencode($input), new __SystemLib\ChunkedGunzipper());
}


<<__EntryPoint>>
function main_inflate_chunked() {
main();
}
