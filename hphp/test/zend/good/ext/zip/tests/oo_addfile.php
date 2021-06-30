<?hh
<<__EntryPoint>>
function main_entry(): void {

  $dirname = dirname(__FILE__) . '/';
  include $dirname . 'utils.inc';
  $file = __SystemLib\hphp_test_tmppath('__tmp_oo_addfile3.zip');

  copy($dirname . 'test.zip', $file);

  $zip = new ZipArchive;
  if (!$zip->open($file)) {
  	exit('failed');
  }
  if (!$zip->addFile($dirname . 'utils.inc', 'test.php')) {
  	echo "failed\n";
  }
  if ($zip->status == ZipArchive::ER_OK) {
  	dump_entries_name($zip);
  	$zip->close();
  } else {
  	echo "failed\n";
  }
  @unlink($file);
}
