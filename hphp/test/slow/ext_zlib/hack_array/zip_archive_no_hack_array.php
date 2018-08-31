<?hh

const string ZIP_NAME = __DIR__ . '/output.zip';
const string OUTPUT_DIR = __DIR__ . '/output';
const varray<string> TARGETS = varray['bing', 'bong'];

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
function main_zip_archive_no_hack_array() {
$opts = darray[
  'add_path' => 1,
  'remove_all_path' => 1,
  'remove_path' => 1,
];

$zip = new ZipArchive();
$zip->open(ZIP_NAME, ZipArchive::CREATE);
$zip->addGlob(__DIR__ . '/test_files/foo*', 0, $opts);
$zip->addPattern('/b[io]ng/', __DIR__ . '/test_files', $opts);
$zip->close();
$zip->open(ZIP_NAME);
stat_archive($zip);
stat_targets($zip);
$zip->extractTo(OUTPUT_DIR, TARGETS);
var_dump(scandir(OUTPUT_DIR));

// Cleanup beyond this point

foreach (TARGETS as $target) {
  unlink(OUTPUT_DIR . "/$target");
}
rmdir(OUTPUT_DIR);
unlink(ZIP_NAME);
}
