<?hh <<__EntryPoint>> function main(): void {
$zip    = zip_open(dirname(__FILE__)."/test_procedural.zip");
$entry  = zip_read($zip);
if (!zip_entry_open($zip, $entry, "r")) exit("Failure");
echo zip_entry_read($entry);
zip_entry_close($entry);
zip_close($zip);
}
