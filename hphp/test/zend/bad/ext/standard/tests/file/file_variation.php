<?php
/* 
 *  Prototype: array file ( string filename [,int use-include_path [,resource context]] );
 *  Description: Reads entire file into an array
                 Returns the  file in an array
 */
require(dirname(__FILE__) . '/file.inc');

$data_array = array( "Garbage data", "Gar\nba\nge d\nata", "Gar\n\nbage \n\n data" );
echo "*** Using various flags values with different data in a file\n";
$count=1;
$file_path = dirname(__FILE__);
foreach( $data_array as $data ) {
   echo "--Iteration $count --\n";
   $fh  = fopen($file_path."/file_variation.tmp", "w");
   fwrite($fh, (binary)$data);
   var_dump( file($file_path."/file_variation.tmp", FILE_IGNORE_NEW_LINES) ); 
   var_dump( file($file_path."/file_variation.tmp", FILE_SKIP_EMPTY_LINES) ); 
   $count++;
   fclose($fh);
}

echo "*** Testing with variation in use_include_path argument ***\n";
$file_path1 = dirname(__FILE__)."/file_variation";
mkdir($file_path1);
ini_set( 'include_path',$file_path.'/file_variation' );

file_put_contents( $file_path1."/file1_variation.tmp", "aaaaaaaaaaaaaaabbbbbbbbbbb111111111222222222" );
var_dump( file("file1_variation.tmp", FILE_USE_INCLUDE_PATH) );
var_dump( file($file_path1."/file1_variation.tmp", 1) );

echo "*** Using file function to remove line containing a key string ***\n";
$file_handle = fopen($file_path."/file2_variation.tmp", "w");
$key = "SEARCH_KEY";
fwrite( $file_handle, (binary)"The key string to be searched is SEARCH_KEY\nLine without key string\nThe key string to be searched is SEARCH_KEY" );
$out_array = file($file_path."/file2_variation.tmp");

echo "File contents in array form Before replacement of the key\n";
var_dump( $out_array );
$file_handle2 = fopen($file_path."/file3_variation.tmp", "w");
// Loop through file content array
foreach($out_array as $line) {
  if( !strstr( $line, $key ) )
    fputs($file_handle2,$line);
}
echo "File contents in array form After replacement of the key\n";
var_dump( file($file_path."/file3_variation.tmp" ));
fclose($file_handle);
fclose($file_handle2);

echo "\n--- Done ---";
?>

<?php
$file_path = dirname(__FILE__);
unlink($file_path."/file_variation.tmp");
unlink($file_path."/file_variation/file1_variation.tmp");
unlink($file_path."/file2_variation.tmp");
unlink($file_path."/file3_variation.tmp");
rmdir($file_path."/file_variation");

?>