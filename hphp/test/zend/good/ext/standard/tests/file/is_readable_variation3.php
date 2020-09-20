<?hh
/* Prototype: bool is_readable ( string $filename );
   Description: Tells whether the filename is readable.
*/

/* test is_executable() with invalid arguments */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_readable(): usage variations ***\n";

$file_handle = fopen(__FILE__, "r");
unset($file_handle);

echo "\n*** Testing is_readable() on miscelleneous filenames ***\n";
$misc_files = varray[
  0,
  1234,
  -2.34555,
  TRUE,
  FALSE,
  NULL,
  " ",
  @varray[],
  @$file_handle
];
/* loop through to test each element in the above array 
   is a readable file */
foreach( $misc_files as $misc_file ) {
  try { var_dump( is_readable($misc_file) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  clearstatcache();
}

echo "Done\n";
}
