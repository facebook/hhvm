<?hh <<__EntryPoint>> function main(): void {
$zip = zip_open(dirname(__FILE__)."/test_procedural.zip");
if (!is_resource($zip)) exit("Failure");
$entries = 0;
$entry = zip_read($zip);
while ($entry) {
  echo zip_entry_compressedsize($entry)."\n";
  $entry = zip_read($zip);
}
zip_close($zip);
}
