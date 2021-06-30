<?hh
<<__EntryPoint>>
function main_entry(): void {
  $dirname = dirname(__FILE__) . '/';
  include $dirname . 'utils.inc';
  $file = __SystemLib\hphp_test_tmppath('__tmp_oo_rename3.zip');

  @unlink($file);

  $zip = new ZipArchive;
  if (!$zip->open($file, ZipArchive::CREATE)) {
  	exit('failed');
  }

  $zip->addFromString('entry1.txt', 'entry #1');
  $zip->addFromString('entry2.txt', 'entry #2');
  $zip->addFromString('dir/entry2d.txt', 'entry #2');

  if (!$zip->status == ZipArchive::ER_OK) {
  	echo "failed to write zip\n";
  }
  $zip->close();

  if (!$zip->open($file)) {
  	exit('failed');
  }


  var_dump($zip->locateName('entry1.txt'));
  var_dump($zip->locateName('eNtry2.txt'));
  var_dump($zip->locateName('eNtry2.txt', ZipArchive::FL_NOCASE));
  var_dump($zip->locateName('enTRy2d.txt', ZipArchive::FL_NOCASE|ZipArchive::FL_NODIR));
  $zip->close();

  @unlink($file);
}
