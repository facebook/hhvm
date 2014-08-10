<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test the effects of touch() on stats of link */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file, link and directory */

$file_name = "$file_path/lstat_stat_variation6.tmp";
$fp = fopen($file_name, "w");  // temp file
fclose($fp);
$link_name = "$file_path/lstat_stat_variation_link6.tmp";
symlink($file_name, $link_name);  // temp link

// touch a link, check stat, there should be difference in atime
echo "*** Testing lstat() for link after using touch() on the link ***\n";
$old_stat = lstat($link_name);
sleep(2);

// clear the cache
clearstatcache();

var_dump( touch($link_name) );

$new_stat = stat($file_name);

// compare self stats
var_dump( compare_self_stat($old_stat) );
var_dump( compare_self_stat($new_stat) );

// compare the stat
$affected_members = array(8, 'atime');
var_dump( compare_stats($old_stat, $new_stat, $affected_members, "<") );
// clear the stat
clearstatcache();

echo "\n--- Done ---";
?>

<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation6.tmp");
unlink("$file_path/lstat_stat_variation_link6.tmp");
?>