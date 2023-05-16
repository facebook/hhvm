<?hh
<<__EntryPoint>>
function main_entry(): void {
  $dirname = dirname(__FILE__) . '/';
  include $dirname . 'utils.inc';
  $file = sys_get_temp_dir().'/'.'__tmp_oo_rename.zip';

  @unlink($file);

  $zip = new ZipArchive;
  if (!$zip->open($file, ZipArchive::CREATE)) {
  	exit('failed');
  }

  $zip->addFromString('entry1.txt', 'entry #1');
  $zip->addFromString('entry2.txt', 'entry #2');
  $zip->addFromString('dir/entry2.txt', 'entry #2');

  if (!$zip->status == ZipArchive::ER_OK) {
  	var_dump($zip);
  	echo "failed\n";
  }

  $zip->close();

  if (!$zip->open($file)) {
  	exit('failed');
  }

  dump_entries_name($zip);
  echo "\n";

  if (!$zip->renameIndex(0, 'ren_entry1.txt')) {
  	echo "failed index 0\n";
  }

  if (!$zip->renameName('dir/entry2.txt', 'dir3/ren_entry2.txt')) {
  	echo "failed name dir/entry2.txt\n";
  }
  dump_entries_name($zip);
  $zip->close();

  @unlink($file);
}
