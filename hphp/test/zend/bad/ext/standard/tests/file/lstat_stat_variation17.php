<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test the effects on stats by changing permissions of a dir */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";

// checking stat() on directory
echo "*** Testing lstat() on a dir after changing its access permission ***\n";
$dirname = "$file_path/lstat_stat_variation17";
mkdir($dirname);

$old_stat = stat($dirname);
sleep(2);
var_dump( chmod($dirname, 0777) );
// clear the stat
clearstatcache();
$new_stat = stat($dirname);
// compare self stats
var_dump( compare_self_stat($old_stat) );
var_dump( compare_self_stat($new_stat) );
// compare the stat
$affected_members = array(2, 10, 'mode', 'ctime');
var_dump( compare_stats($old_stat, $new_stat, $affected_members, "!=") );

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
rmdir("$file_path/lstat_stat_variation17");
?>