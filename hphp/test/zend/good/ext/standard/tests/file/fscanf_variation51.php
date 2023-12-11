<?hh

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan a file for read when file is opened inwrite only mode */
<<__EntryPoint>> function main(): void {

echo "*** Test fscanf(): to read from a file opened in write only mode ***\n"; 

// create a file
$filename = sys_get_temp_dir().'/'.'fscanf_variation51.tmp';
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");
//writing data to the file
@fwrite($file_handle,"sample text\n");

//closing the file
fclose($file_handle);

// various formats
$formats = vec[ "%d", "%f", "%e", "%u", " %s", "%x", "%o"];

$counter = 1;

// various write only modes
$modes = vec["w", "wb", "wt",
               "a", "ab", "at",
               "x", "xb", "xt"
         ];

$counter = 1;
// reading the values from file using different integer formats
foreach($modes as $mode) {
  
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
