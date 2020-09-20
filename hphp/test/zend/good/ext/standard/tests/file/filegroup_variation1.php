<?hh
/* Prototype: int filegroup ( string $filename )
 * Description: Returns the group ID of the file, or FALSE in case of an error.
 */

/* Creating soft and hard links to a file and applying filegroup() on links */
<<__EntryPoint>> function main(): void {
fclose( fopen(__SystemLib\hphp_test_tmppath('filegroup_variation1.tmp'), "w") );

echo "*** Testing filegroup() with links ***\n";
/* With symlink */
symlink(
  __SystemLib\hphp_test_tmppath('filegroup_variation1.tmp'),
  __SystemLib\hphp_test_tmppath('filegroup_variation1_symlink.tmp')
);
var_dump( filegroup(__SystemLib\hphp_test_tmppath('filegroup_variation1_symlink.tmp')) ); //expected true
clearstatcache();

/* With hardlink */
link(
  __SystemLib\hphp_test_tmppath('filegroup_variation1.tmp'),
  __SystemLib\hphp_test_tmppath('filegroup_variation1_link.tmp')
);
var_dump( filegroup(__SystemLib\hphp_test_tmppath('filegroup_variation1_link.tmp')) );  // expected: true
clearstatcache();

echo "\n*** Done ***";

unlink(__SystemLib\hphp_test_tmppath('filegroup_variation1_symlink.tmp'));
unlink(__SystemLib\hphp_test_tmppath('filegroup_variation1_link.tmp'));
unlink(__SystemLib\hphp_test_tmppath('filegroup_variation1.tmp'));
}
