<?hh
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/

/* Testing is_file() with invalid arguments -int, float, bool, NULL, resource */
<<__EntryPoint>> function main(): void {
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
$file_handle = fopen($file_path."/is_file_variation3.tmp", "w");

echo "*** Testing Invalid file types ***\n";
$filenames = varray[
  /* Invalid filenames */
  -2.34555,
  " ",
  "",
  TRUE,
  FALSE,
  NULL,
  $file_handle,
  
  /* scalars */
  1234,
  0
];
   
/* loop through to test each element the above array */
foreach( $filenames as $filename ) {
  try { var_dump( is_file($filename) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  clearstatcache();
}
fclose($file_handle);

echo "\n*** Done ***";
error_reporting(0);
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
unlink($file_path."/is_file_variation3.tmp");
}
