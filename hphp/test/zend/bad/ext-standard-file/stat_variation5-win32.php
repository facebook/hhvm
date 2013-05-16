<?php

/*
 *  Prototype: array stat ( string $filename );
 *  Description: Gives information about a file
 */

/* test the stats of file opened in write mode and then same in read mode */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


$file_handle = fopen("$file_path/stat_variation5.tmp", "w");  // temp file
fclose($file_handle);


echo "\n*** Testing stat(): on a file with read/write permission ***\n";

$filename = "$file_path/stat_variation5.tmp";
$file_handle = fopen($filename, "w");  // create file
fclose($file_handle);
$old_stat = stat($filename);
// clear the stat
clearstatcache();
sleep(2);
// opening file again in read mode
$file_handle = fopen($filename, "r");  // read file
fclose($file_handle);
$new_stat = stat($filename);
// compare self stats
var_dump( compare_self_stat($old_stat) );
var_dump( compare_self_stat($new_stat) );
// compare the stat
$affected_members = array(10, 'ctime');
var_dump( compare_stats($old_stat, $new_stat, $affected_members, "=") );
// clear the stat
clearstatcache();


echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/stat_variation5.tmp");
?>