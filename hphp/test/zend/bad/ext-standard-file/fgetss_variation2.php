<?php
/*
 Prototype: string fgetss ( resource $handle [, int $length [, string $allowable_tags]] );
 Description: Gets line from file pointer and strip HTML tags
*/

// include the common file related test functions 
include ("file.inc");

/*Test fgetss() with all read modes , reading line by line with allowable tags: <test>, <html>, <?> */

echo "*** Testing fgetss() : usage variations  ***\n";

/* string with html and php tags */
$string_with_tags = <<<EOT
<test>Testing fgetss() functions</test>
<?php echo "this string is within php tag"; ?> {;}<{> this
is a heredoc string. <pg>ksklnm@@$$&$&^%&^%&^%&</pg>
<html> html </html> <?php echo "php"; ?>
this line is without any html and php tags
this is a line with more than eighty character,want to check line splitting correctly after 80 characters
this is the text containing \r character 
this text contains some html tags <body> body </body> <br> br </br>
this is the line with \n character. 
EOT;

$filename = dirname(__FILE__)."/fgetss_variation2.tmp"; 

/* try reading the file opened in different modes of reading */
$file_modes = array("r","rb", "rt","r+", "r+b", "r+t");

for($mode_counter = 0; $mode_counter < count($file_modes); $mode_counter++) {
  echo "\n-- Testing fgetss() with file opened using $file_modes[$mode_counter] mode --\n";

  /* create an empty file and write the strings with tags */
  create_file ($filename); //create an empty file
  file_put_contents($filename, $string_with_tags); 
  $file_handle = fopen($filename, $file_modes[$mode_counter]);
  if(!$file_handle) {
    echo "Error: failed to open file $filename!\n";
    exit();
  }

  // rewind the file pointer to beginning of the file
  var_dump( filesize($filename) );
  var_dump( rewind($file_handle) );
  var_dump( ftell($file_handle) );
  var_dump( feof($file_handle) );
   
  /* rewind the file and read the file  line by line with allowable tags */
  echo "-- Reading line by line with allowable tags: <test>, <html>, <?> --\n";
  rewind($file_handle);
  $line = 1;
  while( !feof($file_handle) ) {
     echo "-- Line $line --\n"; $line++;
     var_dump( fgetss($file_handle, 80, "<test>, <html>, <?>") );
     var_dump( ftell($file_handle) );  // check the file pointer position
     var_dump( feof($file_handle) );  // check if eof reached
  }
 
  // close the file 
  fclose($file_handle); 
  // delete the file 
  delete_file($filename);
} // end of for - mode_counter

echo "Done\n";
?>