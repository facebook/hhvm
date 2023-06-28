<?hh

function test($input, $compressed, $decompressor) :mixed{
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

function test_potential_exception($input, $compressed, $decompressor) :mixed{
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

function bzencode($input) :mixed{
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

function test_random_string() :mixed{
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

function test_highly_compressed_string() :mixed{
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

function test_error_on_invalid_input() :mixed{
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

function test_multiple_gzstream() :mixed{
  $filename = tempnam(sys_get_temp_dir(), '');
  $s = "";
  for($i = 0; $i < 256; $i++) {
    $s .= "The quick brown fox jumps over the lazy dog. ";
  }
  $stream = gzencode($s);
  $final_stream = "";
  for($i = 0; $i < 256; $i++) {
    $final_stream .= $stream;
  }

  $handle = fopen($filename, "w");
  fwrite($handle, $final_stream);
  fclose($handle);

  printf("-----\nTesting multiple gz streams\n");
  $handle = fopen($filename, "r");
  $decompressor = new __SystemLib\ChunkedGunzipper();
  $uncompressed_len = 0;
  $stream_count = 1;
  while (!feof($handle)) {
    $chunk = fread($handle, 16384);
    $remaining = strlen($chunk);
    while ($remaining > 0) {
      $uncompressed_len += strlen($decompressor->inflateChunk($chunk));
      $remaining = $decompressor->getUndecompressedByteCount();
      if ($remaining > 0) {
        $offset = strlen($chunk) - $remaining;
        $chunk = substr($chunk, $offset);
      }
      // For sufficiently large compressed chunk and smaller streams,
      // one chunk can span multiple streams
      if ($decompressor->eof()) {
        $decompressor->close();
        if (!feof($handle) || $remaining > 0) {
          $decompressor = new __SystemLib\ChunkedGunzipper();
          $stream_count += 1;
        }
      }
    }
  }
  printf("Uncompressed length: %d, totaling %d streams\n", $uncompressed_len, $stream_count);
}

function main() :mixed{
  test_random_string();
  test_highly_compressed_string();
  test_error_on_invalid_input();
  test_multiple_gzstream();
}

<<__EntryPoint>>
function main_inflate_chunked() :mixed{
main();
}
