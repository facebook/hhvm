<?hh
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link */

/* Testing readlink() with invalid arguments -int, float, bool, NULL, resource */
<<__EntryPoint>> function main(): void {

$file_handle = fopen(sys_get_temp_dir().'/'.'readlink_variation2.tmp', "w");

echo "*** Testing Invalid file types ***\n";
$filenames = vec[
  /* Invalid filenames */
  -2.34555,
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
  try { var_dump( readlink($filename) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  clearstatcache();
}
fclose($file_handle);

echo "\n*** Done ***";

unlink(sys_get_temp_dir().'/'.'readlink_variation2.tmp');
}
