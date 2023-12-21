<?hh

/* Testing fputcsv() to write to a file when default enclosure value and delimiter value
   other than that in the field is provided */
<<__EntryPoint>> function main(): void {
echo "*** Testing fputcsv() : with default enclosure and different delimiter value ***\n";

/* the array is with three elements in it. Each element should be read as 
   1st element is delimiter, 2nd element is enclosure 
   and 3rd element is csv fields
*/
$csv_lists = vec[
  vec[',', '"', vec['water,fruit'] ],
  vec[',', '"', vec['"water","fruit'] ],
  vec[',', '"', vec['"water","fruit"'] ],
  vec[' ', '^', vec['^water^ ^fruit^']],
  vec[':', '&', vec['&water&:&fruit&']],
  vec['=', '=', vec['=water===fruit=']],
  vec['-', '-', vec['-water--fruit-air']],
  vec['-', '-', vec['-water---fruit---air-']],
  vec[':', '&', vec['&""""&:&"&:,:":&,&:,,,,']]

];

$file = sys_get_temp_dir().'/'.'fputcsv_variation12.tmp';

$file_modes = vec["r+", "r+b", "r+t",
                     "a+", "a+b", "a+t",
                     "w+", "w+b", "w+t",
                     "x+", "x+b", "x+t"]; 

$loop_counter = 1;
foreach ($csv_lists as $csv_list) {
  for($mode_counter = 0; $mode_counter < count($file_modes); $mode_counter++) {
    
    echo "\n-- file opened in $file_modes[$mode_counter] --\n";  
    // create the file and add the content with has csv fields
    if ( strstr($file_modes[$mode_counter], "r") ) {
      $fo = new SplFileObject($file, 'w');
    } else {
      $fo = new SplFileObject($file, $file_modes[$mode_counter]);
    }
    $delimiter = $csv_list[0];
    $enclosure = $csv_list[1];
    $csv_field = $csv_list[2];
     
    // write to a file in csv format
    var_dump( $fo->fputcsv($csv_field, '+') );
    // check the file pointer position and eof
    var_dump( $fo->ftell() );
    var_dump( $fo->eof() );
    //close the file
    unset($fo);
    
    // print the file contents 
    var_dump( file_get_contents($file) );

    //delete file
    unlink($file);
  } //end of mode loop 
} // end of foreach

echo "Done\n";
}
