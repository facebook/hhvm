<?php
   $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename2');
   mkdir($targetDir);
   touch($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
   $dir = new DirectoryIterator($targetDir.DIRECTORY_SEPARATOR);
   while(!$dir->isFile()) {
      $dir->next();
   }
   echo $dir->getBasename(array());
?>
<?php error_reporting(0); ?>
<?php
   $targetDir = __DIR__.DIRECTORY_SEPARATOR.md5('directoryIterator::getbasename2');
   unlink($targetDir.DIRECTORY_SEPARATOR.'getBasename_test.txt');
   rmdir($targetDir);
?>