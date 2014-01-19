<?php
   $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename');
   mkdir($targetDir);
   touch($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
   $dir = new DirectoryIterator($targetDir.DIRECTORY_SEPARATOR);
   while(!$dir->isFile()) {
      $dir->next();
   }
   echo $dir->getBasename(array());
?>
<?php
   $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename');
   unlink($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
   rmdir($targetDir);
?>