<?hh
<<__EntryPoint>> function main(): void {
  $file = __SystemLib\hphp_test_tmppath('getBasename_test.txt');
  touch($file);
  $dir = new DirectoryIterator(__SystemLib\hphp_test_tmproot());
  while(!$dir->isFile()) {
    $dir->next();
  }
  echo $dir->getBasename('.txt');

  unlink($file);
}
