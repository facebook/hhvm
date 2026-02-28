<?hh

const string ZIP_NAME = 'output.zip';
const string OUTPUT_DIR = 'output';
const varray<string> TARGETS = vec['bing', 'bong'];
const OPTS = dict[
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
function main() :mixed{
  $zip = new ZipArchive();
  $zip->open(sys_get_temp_dir().'/'.ZIP_NAME, ZipArchive::CREATE);
  $zip->addGlob(__DIR__ . '/test_files/foo*', 0, OPTS);
  $zip->addPattern('/b[io]ng/', __DIR__ . '/test_files', OPTS);
  $zip->close();
  $zip->open(sys_get_temp_dir().'/'.ZIP_NAME);
  stat_archive($zip);
  stat_targets($zip);
  $zip->extractTo(sys_get_temp_dir().'/'.OUTPUT_DIR, TARGETS);
  var_dump(scandir(sys_get_temp_dir().'/'.OUTPUT_DIR));

  // Cleanup
  foreach (TARGETS as $target) {
    unlink(sys_get_temp_dir().'/'.OUTPUT_DIR . "/$target");
  }
  rmdir(sys_get_temp_dir().'/'.OUTPUT_DIR);
  unlink(sys_get_temp_dir().'/'.ZIP_NAME);
}
