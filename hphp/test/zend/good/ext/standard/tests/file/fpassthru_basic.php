<?php
/* 
Prototype: int fpassthru ( resource $handle );
Description: Reads to EOF on the given file pointer from the current position
  and writes the results to the output buffer.
*/

$file_name = dirname(__FILE__)."/passthru.tmp";
$write_handle = fopen($file_name, "w");

$string = "Hello, world\n, abcdefg\tadsdsfdf\n8u2394723947\t$%$%#$%#$%#^#%^ 
          Hello, world\n, abcdefg\tadsdsfdf\n8u2394723947\t$%$%#$%#$%#^#%^\n";
if(substr(PHP_OS, 0, 3) == "WIN")  {
	$string = str_replace("\r",'', $string);
}
fwrite($write_handle, $string);
fclose($write_handle);

$read_handle = fopen($file_name, "r");

echo "*** Test basic functionality of fpassthru() function ***\n";
echo "\n-- Before seek operation --\n";
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 0 --\n";
fseek($read_handle, 0);
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 3 --\n";
fseek($read_handle, 3);
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 13 --\n";
fseek($read_handle, 13);
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 14 --\n";
fseek($read_handle, 14);
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 23 --\n";
fseek($read_handle, 23);
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 34 --\n";
fseek($read_handle, 34);
var_dump( fpassthru($read_handle) );

echo "\n-- After seeking position to 1000 --\n";
fseek($read_handle, 1000);
var_dump( fpassthru($read_handle) );

fclose($read_handle);

echo "*** Done ***\n";

?>
<?php
unlink(dirname(__FILE__)."/passthru.tmp");
?>