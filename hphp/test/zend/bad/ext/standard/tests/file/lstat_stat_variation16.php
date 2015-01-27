<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test the effects on stats with changing permissions of file */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";

$filename = "$file_path/lstat_stat_variation16.tmp";
$fp = fopen($filename, "w");  // temp file
fclose($fp);

// checking stat() on file after changing its permission
echo "*** Testing lstat() on a file after changing its access permission ***\n";
$old_stat = stat($filename);
sleep(2);
var_dump( chmod($filename, 0777) );
// clear the stat
clearstatcache();
$new_stat = stat($filename);
// compare self stats
var_dump( compare_self_stat($old_stat) );
var_dump( compare_self_stat($new_stat) );
// compare the stat
$affected_members = array(10, 'ctime');
var_dump( compare_stats($old_stat, $new_stat, $affected_members, "!=") );

echo "\n--- Done ---";
?>

<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation16.tmp");
?>