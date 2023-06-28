<?hh

// Create test ZIP file
<<__EntryPoint>>
function main_write_after_read_works() :mixed{
  $tempfile = sys_get_temp_dir().'/'.'hello.zip';
  $zip_setup = new ZipArchive();
  $zip_setup->open($tempfile, ZipArchive::OVERWRITE | ZipArchive::CREATE);
  $zip_setup->addFromString('hello.txt', 'Value here');
  $zip_setup->addFromString('hello2.txt', 'Other value here');
  $zip_setup->close();

  // Load the file fresh to modify it
  $zip = new ZipArchive();
  $zip->open($tempfile);

  // Get the 2 strings.
  echo 'First section: ', $zip->getFromIndex(0), "\n";
  echo 'Second section: ', $zip->getFromIndex(1), "\n";
  echo 'Third section: ', var_dump($zip->getFromIndex(2));

  // Remove first.
  echo "\nRemoving first section\n";
  $zip->deleteIndex(0);
  echo 'First section: ', var_dump($zip->getFromIndex(0));
  echo 'Second section: ', $zip->getFromIndex(1), "\n";
  echo 'Third section: ', var_dump($zip->getFromIndex(2));

  $zip->close();
  unlink($tempfile);
}
