<?hh

function test($input, $compressed, $decompressor) {
  $output = '';
  $i = $j = 0;
  $chunks = str_split($compressed, 512 /* bytes */);
  while (!$decompressor->eof()) {
    $chunk = $j < count($chunks) ? $chunks[$j++] : "";
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

function bzencode($input) {
  $filename = tempnam(sys_get_temp_dir(), '');
  try {
    $bz = bzopen($filename, "w");
    bzwrite($bz, $input);
    bzclose($bz);

    $f = fopen($filename, "r");
    $result = "";
    while (!feof($f)) {
      $result .= fread($f, 1024);
    }
    fclose($f);
  } catch (Exception $_) {
  } finally {
    unlink($filename);
  }
  return $result;
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

  printf("-----\nTesting bunzipper\n");
  test($input, bzencode($input), new __SystemLib\ChunkedBunzipper());
}


<<__EntryPoint>>
function main_inflate_chunked() {
main();
}
