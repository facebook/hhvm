<?hh
/* 
 Prototype: array fputcsv ( resource $handle , array $fields [, string $delimiter [, string $enclosure]]] );
 Description: Format line as CSV and write to the file pointer 
*/

/* Testing fputcsv() to write to a file when default enclosure value and delimiter
   of two chars is provided and file is opened in read only mode */
<<__EntryPoint>> function main(): void {
echo "*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***\n";

/* the array is with three elements in it. Each element should be read as 
   1st element is delimiter, 2nd element is enclosure 
   and 3rd element is csv fields
*/
$csv_lists = varray [
  varray[',', '"', varray['water,fruit'] ],
  varray[',', '"', varray['"water","fruit'] ],
  varray[',', '"', varray['"water","fruit"'] ],
  varray[' ', '^', varray['^water^ ^fruit^']],
  varray[':', '&', varray['&water&:&fruit&']],
  varray['=', '=', varray['=water===fruit=']],
  varray['-', '-', varray['-water--fruit-air']],
  varray['-', '-', varray['-water---fruit---air-']],
  varray[':', '&', varray['&""""&:&"&:,:":&,&:,,,,']]

];
$filename = __SystemLib\hphp_test_tmppath('fputcsv_variation14.tmp');

$file_modes = varray ["r", "rb", "rt"];

// create the file
$file_handle = fopen($filename, "w" );
if ( !$file_handle ) {
  echo "Error: failed to create file $filename!\n";
  exit();
}
fclose($file_handle);

$loop_counter = 1;
foreach ($csv_lists as $csv_list) {
  for($mode_counter = 0; $mode_counter < count($file_modes); $mode_counter++) {
    // create the file
    $file_handle = fopen($filename, "w" );
    if ( !$file_handle ) {
      echo "Error: failed to create file $filename!\n";
      exit();
    }
    fclose($file_handle);
    
    echo "\n-- file opened in $file_modes[$mode_counter] --\n";  
    // add the content with has csv fields
      $file_handle = fopen($filename, $file_modes[$mode_counter] );
    if ( !$file_handle ) {
      echo "Error: failed to create file $filename!\n";
      exit();
    }
    $delimiter = $csv_list[0];
    $enclosure = $csv_list[1];
    $csv_field = $csv_list[2];
    
    // write to a file in csv format
    var_dump( fputcsv($file_handle, $csv_field, '++', '%%') );
    // check the file pointer position and eof
    var_dump( ftell($file_handle) );
    var_dump( feof($file_handle) );
    //close the file
    fclose($file_handle);
    
    // print the file contents 
    var_dump( file_get_contents($filename) );

    //delete file
    unlink($filename);
  } //end of mode loop 
} // end of foreach

echo "Done\n";
}
