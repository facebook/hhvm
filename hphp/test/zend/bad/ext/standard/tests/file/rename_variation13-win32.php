<?php
/* Prototype  : bool rename(string old_name, string new_name[, resource context])
 * Description: Rename a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */
/* An array of files */ 
$names_arr = array(
  /* Invalid args */ 
  -1, /* -1 is just a valid filename on windows */
  TRUE, /* 1 as well, (string)TRUE > "1" */
  FALSE,
  NULL,
  "", // I think both p8 and php are wrong on the messages here
  //p8 generates different messages to php, php is probably wrong
  //php has either "File Exists" or "Permission Denied".
  " ",
  "\0",
  
  // as before
  array(),

  /* prefix with path separator of a non existing directory*/
  "/no/such/file/dir", 
  "php/php"

);

/* disable notice so we don't get the array to string conversion notice for "$name" where $name = array() */
error_reporting(E_ALL ^ E_NOTICE);

echo "*** Testing rename() with obscure files ***\n";
$file_path = dirname(__FILE__)."/renameVar13";
$aFile = $file_path.'/afile.tmp';

if (!mkdir($file_path)) {
	die("fail to create $file_path tmp dir");
}

for( $i=0; $i < count($names_arr); $i++ ) {
  $name = $names_arr[$i];
  echo "-- $i testing '$name' " . gettype($name) . " --\n";

  touch($aFile);
  var_dump(rename($aFile, $name));
  if (file_exists($name)) {
     @unlink($name);
  }

  if (file_exists($aFile)) {
     @unlink($aFile);
  }
  var_dump(rename($name, $aFile));
  if (file_exists($aFile)) {
     @unlink($aFile);
  }
}

rmdir($file_path);
echo "\n*** Done ***\n";
?>