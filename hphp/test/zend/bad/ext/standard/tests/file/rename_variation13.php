<?php
/* Prototype  : bool rename(string old_name, string new_name[, resource context])
 * Description: Rename a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing rename() with obscure files ***\n";
$file_path = dirname(__FILE__)."/renameVar13";
$aFile = $file_path.'/afile.tmp';

mkdir($file_path);

/* An array of files */ 
$names_arr = array(
  /* Invalid args */ 
  -1,
  TRUE,
  FALSE,
  NULL,
  "",
  " ",
  "\0",
  array(),

  /* prefix with path separator of a non existing directory*/
  "/no/such/file/dir", 
  "php/php"

);

for( $i=0; $i<count($names_arr); $i++ ) {
  $name = $names_arr[$i];
  echo @"-- testing '$name' --\n";
  touch($aFile);
  var_dump(rename($aFile, $name));
  if (file_exists($name)) {
     unlink($name);
  }
  if (file_exists($aFile)) {
     unlink($aFile);
  }
  var_dump(rename($name, $aFile));
  if (file_exists($aFile)) {
     unlink($aFile);
  }
}

rmdir($file_path);
echo "\n*** Done ***\n";
?>