<?hh

<<__EntryPoint>>
function entrypoint_bug7214(): void {
  $zip = zip_open(dirname(__FILE__)."/binarynull.zip");
  if (!is_resource($zip)) exit("Failure");
  $entries = 0;
  $entry = zip_read($zip);
  $contents = zip_entry_read($entry, zip_entry_filesize($entry));
  if (strlen($contents) == zip_entry_filesize($entry)) {
  	echo "Ok";
  } else {
  	echo "failed";
  }
}
