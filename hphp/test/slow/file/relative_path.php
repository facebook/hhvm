<?php
  error_reporting(-1);
  $dir = "hhvm_file_exists_test";
  mkdir($dir);
  file_put_contents($dir."/a.txt","test");
  var_dump(file_exists($dir."/b/../a.txt"));
  unlink($dir."/a.txt");
  rmdir($dir);
?>
