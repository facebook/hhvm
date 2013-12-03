<?php
/*
   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

$file_path = dirname(__FILE__);
require("$file_path/file.inc");

echo "*** Testing stat() : basic functionality ***\n";

/* creating temp directory and file */

// creating dir
$dirname = "$file_path/stat_basic";
mkdir($dirname);
// stat of the dir created
$dir_stat = stat($dirname);
clearstatcache();
sleep(2);

// creating file
$filename = "$dirname/stat_basic.tmp";
$file_handle = fopen($filename, "w");
fclose($file_handle);
// stat of the file created
$file_stat = stat($filename);
sleep(2);

// now new stat of the dir after file is created
$new_dir_stat = stat($dirname);
clearstatcache();

// stat contains 13 different values stored twice, can be accessed using 
// numeric and named keys, compare them to see they are same  
echo "*** Testing stat(): validating the values stored in stat ***\n";
// Initial stat values
var_dump( compare_self_stat($file_stat) ); //expect true
var_dump( compare_self_stat($dir_stat) );  //expect true

// New stat values taken after creation of file 
var_dump( compare_self_stat($new_dir_stat) );  // expect true

// compare the two stat values, initial stat and stat recorded after 
// creating file, also dump the value of stats
echo "*** Testing stat(): comparing stats (recorded before and after file creation) ***\n";
echo "-- comparing difference in dir stats before and after creating file in it --\n";
$affected_elements = array( 9, 'mtime' );
var_dump( compare_stats($dir_stat, $new_dir_stat, $affected_elements, '!=', true) ); // expect true

echo "*** Testing stat(): for the return value ***\n";
var_dump( is_array( stat($filename) ) );

echo "\n---Done---";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/stat_basic/stat_basic.tmp");
rmdir("$file_path/stat_basic");
?>