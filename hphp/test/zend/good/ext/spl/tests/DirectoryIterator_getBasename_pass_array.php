<?hh
<<__EntryPoint>> function main(): void {
  $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename2');
  mkdir($targetDir);
  touch($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
  $dir = new DirectoryIterator($targetDir.DIRECTORY_SEPARATOR);
  while(!$dir->isFile()) {
    $dir->next();
  }
  try { echo $dir->getBasename(varray[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  error_reporting(0);
  $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename2');
  unlink($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
  rmdir($targetDir);
}
