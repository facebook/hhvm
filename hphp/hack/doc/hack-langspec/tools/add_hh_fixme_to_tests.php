<?hh

// This script automatically adds "HH_FIXME[1002]" statements
// to all the tests that have a main() function so the type
// checker can actually pass AND we can run HHVM tests.

function main(): void {
  $tests_dir = __DIR__ . "/../tests";

  $di = new RecursiveDirectoryIterator($tests_dir,
                                       RecursiveDirectoryIterator::SKIP_DOTS);
  $it = new RecursiveIteratorIterator($di);
  foreach ($it as $test_file) {
    if ($test_file->isFile() && $test_file->getExtension() === "php") {
      $test_contents = file_get_contents($test_file);
      $fixme_pattern = "/HH_FIXME\[1002\] call to main in strict/";
      // Already there. Move on
      if (preg_match($fixme_pattern, $test_contents) === 1) {
        continue;
      }
      $fmain_pattern = "/function[ ]*main[ ]*\([\$\\A-Za-z0-9:\. ]*\)/";
      if (preg_match($fmain_pattern, $test_contents) === 1) {
        $main_call_pattern = "/(\/\/)?[ ]*main[ ]*\(([\$\\A-Za-z0-9:\. ]*)\);/";
        $fixme_str = "/* HH_FIXME[1002] call to main in strict*/" . PHP_EOL
                   . "main($2);"; // $2 is any parameters in $main_call_pattern
        if (preg_match($main_call_pattern, $test_contents) === 1) {
          $test_contents = preg_replace($main_call_pattern, $fixme_str,
                                        $test_contents);
        } else {
          $test_contents .= PHP_EOL . $fixme_str;
        }
      }
      file_put_contents($test_file, $test_contents);
    }
  }
}

main();
