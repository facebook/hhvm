<?hh
/*
 Prototype: string fgetc ( resource $handle );
 Description: Gets character from file pointer
*/

/* read from fie using fgetc, file opened using different
   read read modes */
<<__EntryPoint>> function main(): void {
echo "*** Testing fgetc() : usage variations ***\n";
echo "-- Testing fgetc() with files opened with different read modes --\n";

$file_modes = vec[ "a+", "a+b", "a+t", 
                     "x+", "x+b", "x+t", 
                     "w+", "w+b", "w+t" ];

$filename = sys_get_temp_dir().'/'.'fgetc_variation4.tmp';
foreach ($file_modes as $file_mode ) {
  echo "-- File opened in mode : $file_mode --\n";

  $file_handle = fopen($filename, $file_mode);
  if(!$file_handle) {
    echo "Error: failed to open file $filename!\n";
    exit();
  }
  $data = "fgetc\n test";
  fwrite($file_handle, $data);

  // rewind the file pointer to beginning of the file
  var_dump( rewind($file_handle) ); 
  var_dump( ftell($file_handle) ); 
  var_dump( feof($file_handle) );

  // read from file, at least 7 chars
  for($counter =0; $counter < 7; $counter ++) {
    var_dump( fgetc($file_handle) ); // expected : 1 char
    var_dump( ftell($file_handle) );
    var_dump( feof($file_handle) ); // check if end of file pointer is set
  }

  // close the file
  fclose($file_handle);

  // delete the file
  unlink($filename); 
}

echo "Done\n";
}
