<?hh

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan an empty file */
<<__EntryPoint>> function main(): void {

echo "*** Test fscanf(): to read an empty file ***\n"; 

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
  $filename = sys_get_temp_dir().'/'.'fscanf_variation52.tmp';
  $file_handle = fopen($filename, "w");
  if($file_handle == false)
    exit("Error:failed to open file $filename");
  //closing the file
  fclose($file_handle);

  // opening file in $mode mode
  $file_handle = fopen($filename, $mode);
  if($file_handle == false) {
    exit("Error:failed to open file $filename");
  }
  echo "\n-- iteration $counter --\n";

  foreach($formats as $format) {
    var_dump( fscanf($file_handle,$format) );
    rewind($file_handle);
  }
  $counter++;
  fclose($file_handle);
  unlink($filename);
}

echo "\n*** Done ***";
}
