<?hh
<<__EntryPoint>> function main(): void {

  $file = sys_get_temp_dir().'/'.'getBasename_test.txt';
  touch($file);
  $dir = new DirectoryIterator(sys_get_temp_dir());
  while(!$dir->isFile()) {
    $dir->next();
  }
  try { echo $dir->getBasename(vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  unlink($file);
}
