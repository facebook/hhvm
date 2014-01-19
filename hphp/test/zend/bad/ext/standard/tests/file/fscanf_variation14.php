<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan different strings using different string format types */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): different string format types with different string ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation14.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

// different valid strings

/* string created using Heredoc (<<<) */
$heredoc_string = <<<EOT
This is string defined
using heredoc.
EOT;
/* heredoc string with only numerics */
$heredoc_numeric_string = <<<EOT
123456 3993
4849 string
EOT;
/* null heardoc string */
$heredoc_empty_string = <<<EOT
EOT;
$heredoc_null_string = <<<EOT
NULL
EOT;

$valid_strings = array(
  "",
  " ",
  '',
  ' ',
  "string",
  'string',
  "NULL",
  'null',
  "FALSE",
  'true',
  "\x0b",
  "\0",
  '\0',
  '\060',
  "\070",
  "0x55F",
  "055",
  "@#$#$%%$^^$%^%^$^&",
  $heredoc_string,
  $heredoc_numeric_string,
  $heredoc_empty_string,
  $heredoc_null_string
);

// various string formats
$string_formats = array( "%s", 
                         "%hs", "%ls", "%Ls",
                         " %s", "%s ", "% s",
                         "\t%s", "\n%s", "%4s",
                         "%30s", "%[a-zA-Z0-9]", "%*s"
                  );

$counter = 1;

// writing to the file
foreach($valid_strings as $string) {
  @fprintf($file_handle, $string);
  @fprintf($file_handle, "\n");
}
// closing the file
fclose($file_handle);

// opening the file for reading
$file_handle = fopen($filename, "r");
if($file_handle == false) {
  exit("Error:failed to open file $filename");
}

$counter = 1;
// reading the values from file using different string formats
foreach($string_formats as $string_format) {
  // rewind the file so that for every foreach iteration the file pointer starts from bof
  rewind($file_handle);
  echo "\n-- iteration $counter --\n";
  while( !feof($file_handle) ) {
    var_dump( fscanf($file_handle,$string_format) );
  }
  $counter++;
}

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation14.tmp";
unlink($filename);
?>