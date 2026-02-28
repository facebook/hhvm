<?hh
/* Prototype: bool is_executable ( string $filename );
   Description: Tells whether the filename is executable
*/

/* test is_executable() with invalid arguments */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_executable(): usage variations ***\n";

echo "\n*** Testing is_executable() on invalid files ***\n";
$invalid_files = vec[
  " ",
];
/* loop through to test each element in the above array
   is an executable file */
foreach( $invalid_files as $invalid_file ) {
  try { var_dump( is_executable($invalid_file) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  clearstatcache();
}

echo "Done\n";
}
