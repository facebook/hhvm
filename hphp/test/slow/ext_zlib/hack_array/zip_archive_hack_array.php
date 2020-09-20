<?hh

const string ZIP_NAME = 'output.zip';
const string OUTPUT_DIR = 'output';
const varray<string> TARGETS = varray['bing', 'bong'];
const OPTS = darray[
  'add_path' => 1,
  'remove_all_path' => 1,
  'remove_path' => 1,
];

function stat_archive(ZipArchive $zip): void {
  var_dump($zip->numFiles);
  for ($i = 0; $i < $zip->numFiles; $i++) {
    var_dump($zip->statIndex($i));
  }
}

function stat_targets(ZipArchive $zip): void {
  foreach (TARGETS as $target) {
    var_dump($zip->statName($target));
  }
}

<<__EntryPoint>>
function main() {
  $zip = new ZipArchive();
  $zip->open(__SystemLib\hphp_test_tmppath(ZIP_NAME), ZipArchive::CREATE);
  $zip->addGlob(__DIR__ . '/test_files/foo*', 0, OPTS);
  $zip->addPattern('/b[io]ng/', __DIR__ . '/test_files', OPTS);
  $zip->close();
  $zip->open(__SystemLib\hphp_test_tmppath(ZIP_NAME));
  stat_archive($zip);
  stat_targets($zip);
  $zip->extractTo(__SystemLib\hphp_test_tmppath(OUTPUT_DIR), TARGETS);
  var_dump(scandir(__SystemLib\hphp_test_tmppath(OUTPUT_DIR)));

  // Cleanup
  foreach (TARGETS as $target) {
    unlink(__SystemLib\hphp_test_tmppath(OUTPUT_DIR . "/$target"));
  }
  rmdir(__SystemLib\hphp_test_tmppath(OUTPUT_DIR));
  unlink(__SystemLib\hphp_test_tmppath(ZIP_NAME));
}
