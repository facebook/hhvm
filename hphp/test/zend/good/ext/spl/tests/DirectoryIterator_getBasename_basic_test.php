<?hh
<<__EntryPoint>> function main(): void {
  $file = sys_get_temp_dir().'/'.'getBasename_test.txt';
  touch($file);
  $dir = new DirectoryIterator(sys_get_temp_dir());
  while(!$dir->isFile()) {
    $dir->next();
  }
  echo $dir->getBasename('.txt');

  unlink($file);
}
