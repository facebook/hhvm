<?hh <<__EntryPoint>> function main(): void {
$zip = zip_open(dirname(__FILE__)."/test_procedural.zip");
if (!is_resource($zip)) exit("Failure");
$entries = 0;
while ($entry = zip_read($zip)) {
  echo zip_entry_filesize($entry)."\n";
}
zip_close($zip);
}
