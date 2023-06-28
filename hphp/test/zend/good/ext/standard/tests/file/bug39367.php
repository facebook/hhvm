<?hh
function test() :mixed{
  $f1 = sys_get_temp_dir() . '/1link';
  $f2 = sys_get_temp_dir() . '/1tmp';
  $f3 = sys_get_temp_dir() . '/testfile1';
  unlink($f1);
  unlink($f2);
  unlink($f3);

  file_put_contents($f3, 'ok');
  symlink($f3, $f2);
  rename($f2, $f1);
  echo file_get_contents($f1)."\n";

  unlink($f1);
  clearstatcache(true);

  echo file_get_contents($f1)."\n";

  unlink($f1);
  unlink($f2);
  unlink($f3);
}
<<__EntryPoint>> function main(): void {
@test();
}
