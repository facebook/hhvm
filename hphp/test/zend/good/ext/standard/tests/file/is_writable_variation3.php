<?hh
/* Prototype: bool is_writable ( string $filename );
   Description: Tells whether the filename is writable.

   is_writeable() is an alias of is_writable()
*/

/* test is_writable() & is_writeable() with invalid arguments */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_writable(): usage variations ***\n";

echo "\n*** Testing is_writable() with invalid filenames ***\n";
$misc_files = vec[
  " ",
];
/* loop through to test each element in the above array
   is a writable file */
foreach( $misc_files as $misc_file ) {
  try { var_dump( is_writable($misc_file) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( is_writeable($misc_file) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  clearstatcache();
}

echo "Done\n";
}
