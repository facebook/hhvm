<?hh

// Replace absolute paths with regex paths for now.
// Could add other checks here too.
function check_output(array<string> $output, string $test_file): array<string> {
  $len = count($output);
  $rp_test_file = realpath($test_file);
  // /data/users/joelm/hack-langspec/tests/Script_Inclusion/require.php
  $pattern = "/\/[A-Z0-9a-z\.\-_#\/]+(\/tests\/[A-Z0-9a-z\.\-_#\/]+\.php)/";
  // Replace every element in the $output array %s/tests/ ... .php
  // where $1 is the parenthesized part of the regex we actually want in
  // expectf files.
  $replace = "%s$1";
  $output = preg_replace($pattern, $replace, $output);
  return $output;
}

function main(?string $replace_option = null): void {
  $replace_all = false;
  if ($replace_option === 'replace') {
    $replace_all = true;
  }
  $tests_dir = __DIR__ . "/../tests";

  $di = new RecursiveDirectoryIterator($tests_dir,
                                       RecursiveDirectoryIterator::SKIP_DOTS);
  $it = new RecursiveIteratorIterator($di);
  $output = array();
  foreach ($it as $test_file) {
    if ($test_file->isFile() && $test_file->getExtension() === "php") {
      if ($replace_all ||
          (!file_exists($test_file . ".expect") &&
           !file_exists($test_file . ".expectf"))) {
        exec("hhvm " . $test_file . " 2>&1", $output);
        $updated_output = check_output($output, $test_file->getRealPath());
        $file_output = implode(PHP_EOL, $updated_output);
        $expect_file_name = $test_file . ".expect";
        if ($updated_output !== $output) { // .expectf file since output changed
           $expect_file_name = $test_file . ".expectf";
        }
        file_put_contents($expect_file_name, $file_output);
        echo "Creating " . $expect_file_name . PHP_EOL;
      }
    }
  }
}

if (count($argv) === 2) {
  main($argv[1]);
} else {
  main();
}
