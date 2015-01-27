<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test the effects of touch() on stats of file */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file  */

$file_name = "$file_path/lstat_stat_variation4.tmp";
$fp = fopen($file_name, "w");  // temp file
fclose($fp);

// touch a file check stat, there should be difference in atime
echo "*** Testing stat() for file after using touch() on the file ***\n";
$old_stat = stat($file_name);
// clear the cache
clearstatcache();
sleep(2);
var_dump( touch($file_name) );
$new_stat = stat($file_name);

// compare self stats
var_dump( compare_self_stat($old_stat) );
var_dump( compare_self_stat($new_stat) );

// compare the stat
$affected_members = array(8, 'atime');
var_dump( compare_stats($old_stat, $new_stat, $affected_members, "<") );
// clear the cache
clearstatcache();

echo "\n--- Done ---";
?>

<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation4.tmp");
?>