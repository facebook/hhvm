<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test the effects of is_link() on stats of hard link */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file & link */
$filename = "$file_path/lstat_stat_variation14.tmp";
$fp = fopen($filename, "w");  // temp file
fclose($fp);

echo "*** Checking lstat() and stat() on hard link ***\n";
$linkname = "$file_path/lstat_stat_variation14_hard.tmp";
//ensure that link doesn't exists 
@unlink($linkname);

// create the link
var_dump( link($filename, $linkname) );
$file_stat = stat($filename);
$link_stat = lstat($linkname);
// compare self stats
var_dump( compare_self_stat($file_stat) );
var_dump( compare_self_stat($link_stat) );
// compare the stat
var_dump( compare_stats($file_stat, $link_stat, $all_stat_keys) );
// clear the stat
clearstatcache();

echo "\n--- Done ---";
?>

<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation14_hard.tmp");
unlink("$file_path/lstat_stat_variation14.tmp");
?>