<?hh

// Create test ZIP file
<<__EntryPoint>>
function main_write_after_read_works() :mixed{
  $tempfile = sys_get_temp_dir().'/'.'hello.zip';
  $zip_setup = new ZipArchive();
  $zip_setup->open($tempfile, ZipArchive::OVERWRITE | ZipArchive::CREATE);
  $zip_setup->addFromString('hello.txt', 'Value here');
  $zip_setup->close();
  $zip = new ZipArchive();
  $zip->open($tempfile);
  echo 'value: ', $zip->getFromIndex(0, 0x100000000 + 4096), "\n";
  $zip->close();
  unlink($tempfile);
}
