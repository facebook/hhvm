<?hh

/**
 * The description of mailparse_uudecode_all gives the impression that it can
 * handle N = [0, inf) uuencoded files in a stream.
 *
 * However, it seems to only correctly handle exactly 1 file. This test
 * demonstrates the expected behavior of various input scenarios, where
 * 'expected' is defined as the behavior that exists in php's mail-mailparse
 * package... not what a human might actually expect.
 */

function describe(darray<string, mixed> $info): void {
  $ofname = "origfilename not set";
  if (array_key_exists('origfilename', $info)) {
    $ofname = $info['origfilename'];
  }

  echo(sprintf("<%s> =>\n", $ofname));

  readfile($info['filename']);

  echo("\n\n");
}

function process(string $name, string $desc): void {
  echo(sprintf("== Processing %s ==\n# %s\n\n", $name, $desc));

  $filepath = __DIR__ . "/uue_data/" . $name . ".uue";

  $fp = null;
  try{
    $fp = fopen($filepath, 'r');
    if (!$fp) {
      echo(sprintf("Error: File not found: %s\n", $filepath));
      return;
    }
    $data = mailparse_uudecode_all($fp);

    if (!$data) {
      echo("Error: no data!\n");
      return;
    }

    foreach ($data as $info) {
      describe($info);
      unlink($info['filename']);
    }
  } catch (Exception $e) {
    echo(sprintf("Exception: %s\n", $e->getMessage()));
  } finally {
    if ($fp !== null) {
      fclose($fp);
    }
  }
}

// for x in scenarios process(k, v)


<<__EntryPoint>>
function main_ext_uudecode() :mixed{
$scenarios = Map<string,string>{
  "simple" => "exactly one uuencoded file, no additional stuff",
  "mixed" => "literal data, and then one uuencoded file (common)",
  "mixed_ws" => "'mixed' with whitespace after the name",
  "mixed_onechar" => "'mixed' with a 1-character file name",
  "mixed_nochar" => "'mixed' with a 0-character file name",
  "plaintext" => "no actual uuencoded file",
  "twofile" => "literal data, and TWO uuencoded files",
  "complex" => "literal data, one uuencoded file, then more data.",
  "badbegin" => "mixed with a malformed 'begin' line",
  "badbegin2" => "twofile with a malformed 'begin' line on file 2",
  "badbegin3" => "twofile with a malformed 'begin' line on file 1",
};

foreach ($scenarios as $name => $desc) {
  process($name, $desc);
}
}
