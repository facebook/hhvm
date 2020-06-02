<?hh
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/

/* Creating soft and hard links to a file and applying is_file() on links */ 
<<__EntryPoint>> function main(): void {
fclose( fopen(__SystemLib\hphp_test_tmppath('is_file_variation2.tmp'), "w") );

echo "*** Testing is_file() with links ***\n";
/* With symlink */
symlink(
  __SystemLib\hphp_test_tmppath('is_file_variation2.tmp'),
  __SystemLib\hphp_test_tmppath('is_file_variation2_symlink.tmp')
);
var_dump( is_file(__SystemLib\hphp_test_tmppath('is_file_variation2_symlink.tmp')) ); //expected true
clearstatcache();

/* With hardlink */
link(
  __SystemLib\hphp_test_tmppath('is_file_variation2.tmp'),
  __SystemLib\hphp_test_tmppath('is_file_variation2_link.tmp')
);
var_dump( is_file(__SystemLib\hphp_test_tmppath('is_file_variation2_link.tmp')) );  // expected: true
clearstatcache();

echo "\n*** Done ***";

unlink(__SystemLib\hphp_test_tmppath('is_file_variation2_symlink.tmp'));
unlink(__SystemLib\hphp_test_tmppath('is_file_variation2_link.tmp'));
unlink(__SystemLib\hphp_test_tmppath('is_file_variation2.tmp'));
}
