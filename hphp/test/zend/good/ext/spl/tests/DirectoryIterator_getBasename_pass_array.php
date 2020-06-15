<?hh
<<__EntryPoint>> function main(): void {

  $file = __SystemLib\hphp_test_tmppath('getBasename_test.txt');
  touch($file);
  $dir = new DirectoryIterator(__SystemLib\hphp_test_tmproot());
  while(!$dir->isFile()) {
    $dir->next();
  }
  try { echo $dir->getBasename(varray[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  unlink($file);
}
