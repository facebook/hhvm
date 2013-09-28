<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

$file_path = dirname(__FILE__);
require "$file_path/file.inc";

/* test the effects on stats with writing data into a  file */

$file_name = "$file_path/lstat_stat_variation7.tmp";
$fp = fopen($file_name, "w");  // temp file
fclose($fp);

// writing to an empty file
echo "*** Testing stat() on file after data is written in it ***\n";
$fh = fopen($file_name,"w");
$old_stat = stat($file_name);
clearstatcache();
fwrite($fh, str_repeat((binary)"Hello World", $old_stat['blksize']));
$new_stat = stat($file_name);

// compare self stats
var_dump( compare_self_stat($old_stat) );
var_dump( compare_self_stat($new_stat) );
// compare the stats
$comp_arr = array(7, 12, 'size', 'blocks');
var_dump(compare_stats($old_stat, $new_stat, $comp_arr, "<"));
clearstatcache();

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation7.tmp");
?>