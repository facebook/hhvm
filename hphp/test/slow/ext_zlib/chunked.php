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

function test_potential_exception($input, $compressed, $decompressor) {
  $output = '';
  $i = $j = 0;
  $chunks = str_split($compressed, 16384);
  $errs = 0;
  while (!$decompressor->eof()) {
    $chunk = $j < count($chunks) ? $chunks[$j++] : "";
    try {
      $this_chunk = $decompressor->inflateChunk($chunk);
    } catch (Exception $e) {
      printf("inflateChunk() threw an exception: %s(\"%s\")\n", get_class($e), $e->getMessage());
      $decompressor->close();
      return;
    }
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

function test_random_string() {
  $input = 'herp derp';
  // need a bigger string, with big output
  for ($i = 0; $i <= 1024; ++$i) {
    $input .= crc32($input);
  }

  printf("-----\nTesting inflator with random string\n");
  test($input, gzdeflate($input), new __SystemLib\ChunkedInflator());

  printf("-----\nTesting gunzipper with random string\n");
  test($input, gzencode($input), new __SystemLib\ChunkedGunzipper());

  printf("-----\nTesting bunzipper with random string\n");
  test($input, bzencode($input), new __SystemLib\ChunkedBunzipper());
}

function test_highly_compressed_string() {
  // 32MB of zeroes
  $input = 'a';
  for ($i = 0; $i <= 24; ++$i) {
    $input .= $input;
  }

  printf("-----\nTesting inflator with highly compressed string\n");
  test_potential_exception($input, gzdeflate($input), new __SystemLib\ChunkedInflator());

  printf("-----\nTesting gunzipper with highly compressed string\n");
  test_potential_exception($input, gzencode($input), new __SystemLib\ChunkedGunzipper());

  printf("-----\nTesting bunzipper with highly compressed string\n");
  test_potential_exception($input, bzencode($input), new __SystemLib\ChunkedBunzipper());
}

function test_error_on_invalid_input() {
  try {
    printf("-----\nTesting inflator exception\n");
    $decompressor = new __SystemLib\ChunkedInflator();
    $this_chunk = $decompressor->inflateChunk("Invalid input");
    printf("inflateChunk() did not throw an exception\n");
  } catch (Exception $e) {
    printf("inflateChunk() threw an exception: %s(\"%s\")\n", get_class($e), $e->getMessage());
  }
  try {
    printf("-----\nTesting gunzipper exception\n");
    $decompressor = new __SystemLib\ChunkedGunzipper();
    $this_chunk = $decompressor->inflateChunk("Invalid input");
    printf("inflateChunk() did not throw an exception\n");
  } catch (Exception $e) {
    printf("inflateChunk() threw an exception: %s(\"%s\")\n", get_class($e), $e->getMessage());
  }
  try {
    printf("-----\nTesting bunzipper exception\n");
    $decompressor = new __SystemLib\ChunkedBunzipper();
    $this_chunk = $decompressor->inflateChunk("Invalid input");
    printf("inflateChunk() did not throw an exception\n");
  } catch (Exception $e) {
    printf("inflateChunk() threw an exception: %s(\"%s\")\n", get_class($e), $e->getMessage());
  }
}

function main() {
  test_random_string();
  test_highly_compressed_string();
  test_error_on_invalid_input();
}

<<__EntryPoint>>
function main_inflate_chunked() {
main();
}
