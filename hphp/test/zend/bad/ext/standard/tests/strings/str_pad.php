<?php
/* Pad a string to a certain length with another string */

echo "\n#### Basic operations ####\n";
$input_string = "str_pad()";
$pad_length = 20;
$pad_string = "-+";
var_dump( str_pad($input_string, $pad_length) ); // default pad_string & pad_type
var_dump( str_pad($input_string, $pad_length, $pad_string) ); // default pad_type
var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_LEFT) ); 
var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_RIGHT) );
var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_BOTH) );

echo "\n#### variations with input string and pad-length ####\n";
/* different input string variation */
$input_strings = array( 
		   "variation", // normal string
                   chr(0).chr(255).chr(128).chr(234).chr(143), 	// >7-bit ASCII
		   "", 	// empty string
                   NULL,  // NULL
                   true,  // boolean 
                   15,  // numeric
                   15.55,  // numeric
                   "2990"  // numeric string
                 );
/* different pad_lengths */
$pad_lengths = array(
		 -PHP_INT_MAX,  // huge negative value
		 -1,  // negative value
                 0,  // pad_length < sizeof(input_string)
                 9,  // pad_length <= sizeof(input_string)
                 10,  // pad_length > sizeof(input_string) 
                 16,  // pad_length > sizeof(input_string)
               );
$pad_string = "=";
/*loop through to use each varient of $pad_length on
  each element of $input_strings array */
foreach ($input_strings as $input_string ) {
  foreach ($pad_lengths as $pad_length ) {
    var_dump( str_pad($input_string, $pad_length) ); // default pad_string & pad_type
    var_dump( str_pad($input_string, $pad_length, $pad_string) ); // default pad_type
    var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_LEFT) ); 
    var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_RIGHT) );
    var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_BOTH) );
  }
}

echo "\n#### variation with pad string ####\n";
$pad_strings = array ("=", 1, true, "string_pad", 1.5, "\t", '\t');
$input_string="variation";
$pad_length = 16;
foreach ( $pad_strings as $pad_string ) {
    var_dump( str_pad($input_string, $pad_length, $pad_string) ); // default pad_type
    var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_LEFT) ); 
    var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_RIGHT) );
    var_dump( str_pad($input_string, $pad_length, $pad_string, STR_PAD_BOTH) );
}

echo "\n#### error conditions ####";
/* args less than min. expected of 2 */
str_pad();
str_pad($input_string);

/* args more than expected,expected 4 */
str_pad($input_tring, $pad_length, $pad_string, STR_PAD_LEFT, NULL );

echo "\n--- padding string as null ---";
str_pad($input_string, 12, NULL);
str_pad($input_string, 12, "");

/* bad pad_type - passing an undefined one */
var_dump( str_pad($input_string, $pad_length, "+", 15) );

echo "Done\n";
?>