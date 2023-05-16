<?hh
/* Prototype: int fileowner ( string $filename )
 * Description: Returns the user ID of the owner of the file, or
 *              FALSE in case of an error.
 */

/* Creating soft and hard links to a file and applying fileowner() on links */
<<__EntryPoint>> function main(): void {
fclose( fopen(sys_get_temp_dir().'/'.'fileowner_variation1.tmp', "w")) ;

echo "*** Testing fileowner() with links ***\n";
/* With symlink */
symlink(
  sys_get_temp_dir().'/'.'fileowner_variation1.tmp',
  sys_get_temp_dir().'/'.'fileowner_variation1_symlink.tmp'
);
var_dump( fileowner(sys_get_temp_dir().'/'.'fileowner_variation1_symlink.tmp')) ; //expected true
clearstatcache();

/* With hardlink */
link(
  sys_get_temp_dir().'/'.'fileowner_variation1.tmp',
  sys_get_temp_dir().'/'.'fileowner_variation1_link.tmp'
);
var_dump( fileowner(sys_get_temp_dir().'/'.'fileowner_variation1_link.tmp')) ;  // expected: true
clearstatcache();

echo "\n*** Done ***";

unlink(sys_get_temp_dir().'/'.'fileowner_variation1_symlink.tmp');
unlink(sys_get_temp_dir().'/'.'fileowner_variation1_link.tmp');
unlink(sys_get_temp_dir().'/'.'fileowner_variation1.tmp');
}
