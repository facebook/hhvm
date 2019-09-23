<?hh
<<__EntryPoint>> function main(): void {
  $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename1');
  mkdir($targetDir);
  touch($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
  $dir = new DirectoryIterator($targetDir.DIRECTORY_SEPARATOR);
  while(!$dir->isFile()) {
    $dir->next();
  }
  echo $dir->getBasename('.txt');
  error_reporting(0);
  $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename1');
  unlink($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
  rmdir($targetDir);
}
