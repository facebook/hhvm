<?php
/* Prototype: bool is_dir ( string $dirname );
   Description: Tells whether the dirname is a directory
     Returns TRUE if the dirname exists and is a directory, FALSE  otherwise.
*/

/* Passing invalid arguments to is_dir() */

$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
$dir_handle = opendir( $file_path );

echo "*** Testing is_dir() with Invalid arguments: expected bool(false) ***\n";
$dirnames = array(
  /* Invalid dirnames */
  -2.34555,
  TRUE,
  FALSE,
  NULL,
  " ",
  $dir_handle,

  /* scalars */
  0,
  1234
);

/* loop through to test each element the above array */
foreach($dirnames as $dirname) {
  try { var_dump( is_dir($dirname) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
closedir($dir_handle);

echo "\n*** Done ***";
?>
