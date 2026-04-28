<?hh

<<__EntryPoint>>
function main() :mixed{
  $tempfile = sys_get_temp_dir().'/'.'hello.zip';
  $zip_setup = new ZipArchive();
  $zip_setup->open($tempfile, ZipArchive::OVERWRITE | ZipArchive::CREATE);
  $zip_setup->addFromString('hello.txt', 'value');
  $zip_setup->close();
  $zip = new ZipArchive();
  $zip->open($tempfile);
  echo 'value: ', $zip->getFromName('hello.txt', 0x100000000 + 4096), "\n";
  unlink($tempfile);
}
