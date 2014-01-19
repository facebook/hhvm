<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan a file to read objects */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): to read objects from a file ***\n"; 

// declare a class
class foo
{
  public $var1 = 1;
  public $var2 = 2;
  function __toString() {
    return "Object";
  }
}

// create an object
$obj = new foo(); //creating new object

// create a file
$filename = "$file_path/fscanf_variation54.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");
//writing object to the file
fwrite($file_handle, $obj);

//closing the file
fclose($file_handle);

// various formats
$formats = array( "%d", "%f", "%e", "%u", " %s", "%x", "%o");

$counter = 1;

// opening file for read  
$file_handle = fopen($filename, "r");
  if($file_handle == false) {
    exit("Error:failed to open file $filename");
  }
echo "\n-- iteration $counter --\n";

foreach($formats as $format) {
  var_dump( fscanf($file_handle,$format) );
  rewind($file_handle);
}

fclose($file_handle);
echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation54.tmp";
unlink($filename);
?>