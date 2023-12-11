<?hh

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to read a file when file pointer is pointing to EOF */
<<__EntryPoint>> function main(): void {

echo "*** Test fscanf(): to read a file when file pointer is pointing to EOF ***\n"; 

// various formats
$formats = vec[ "%d", "%f", "%e", "%u", " %s", "%x", "%o"];

$counter = 1;

// various read modes
$modes = vec["r", "rb", "rt", "r+", "r+b", "r+t",
               "w+", "w+b", "w+t",
               "a+", "a+b", "a+t"
         ];

$counter = 1;
// reading the values from file using different integer formats
foreach($modes as $mode) {
  
  // create an empty file
  $filename = sys_get_temp_dir().'/'.'fscanf_variation53.tmp';
  $file_handle = fopen($filename, "w");
  if($file_handle == false)
    exit("Error:failed to open file $filename");
 
  //writing data to the file
  @fwrite($file_handle, "Sample text\n");
  
  // writing a blank line
  @fwrite($file_handle, "\n");

  //closing the file
  fclose($file_handle);

  // opening file in $mode mode
  $file_handle = fopen($filename, $mode);
  if($file_handle == false) {
    exit("Error:failed to open file $filename");
  }
  echo "\n-- iteration $counter --\n";
 
  // current location
  var_dump( ftell($file_handle) );
 
  // set the file pointer to eof
  var_dump( fseek($file_handle, 0, SEEK_END) );
  
  // current location  
  var_dump( ftell($file_handle) );

  foreach($formats as $format) {
    var_dump( fscanf($file_handle,$format) );
  }
  $counter++;
  fclose($file_handle);
  unlink($filename);
}

echo "\n*** Done ***";
}
