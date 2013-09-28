<?php

$var_array = array(
                   array(),
                   array(1,2,3,4,5,6,7,8,9),
                   array("One", "Two", "Three", "Four", "Five"),
                   array(6, "six", 7, "seven", 8, "eight", 9, "nine"),
                   array( "a" => "aaa", "A" => "AAA", "c" => "ccc", "d" => "ddd", "e" => "eee"),
                   array("1" => "one", "2" => "two", "3" => "three", "4" => "four", "5" => "five"),
                   array(1 => "one", 2 => "two", 3 => 7, 4 => "four", 5 => "five"),
                   array("f" => "fff", "1" => "one", 4 => 6, "" => "blank", 2.4 => "float", "F" => "FFF",
                         "blank" => "", 3.7 => 3.7, 5.4 => 7, 6 => 8.6, '5' => "Five"),
                   array(12, "name", 'age', '45'),
                   array( array("oNe", "tWo", 4), array(10, 20, 30, 40, 50), array())
                 );

$num = 4;
$str = "john";

/* Zero args */
echo"\n*** Output for Zero Argument ***\n";
array_slice();

/* Single args */
echo"\n*** Output for Single array Argument ***\n";
array_slice($var_array);

/* More than valid no. of args (ie. >4 )  */
echo"\n*** Output for invalid number of Arguments ***\n";
array_slice($var_array, 2, 4, true, 3);

/* Scalar arg */
echo"\n*** Output for scalar Argument ***\n";
array_slice($num, 2);

/* String arg */
echo"\n*** Output for string Argument ***\n";
array_slice($str, 2);

$counter = 1;
foreach ($var_array as $sub_array)
{
  /* variations with two arguments */
  /* offset values >, < and = 0    */

  echo"\n*** Iteration ".$counter." ***\n";
  echo"\n*** Variation with first two Arguments ***\n";
  var_dump ( array_slice($sub_array, 1) );
  var_dump ( array_slice($sub_array, 0) );
  var_dump ( array_slice($sub_array, -2) );

  /* variations with three arguments */
  /* offset value variations with length values  */
  echo"\n*** Variation with first three Arguments ***\n";
  var_dump ( array_slice($sub_array, 1, 3) );
  var_dump ( array_slice($sub_array, 1, 0) );
  var_dump ( array_slice($sub_array, 1, -3) );
  var_dump ( array_slice($sub_array, 0, 3) );
  var_dump ( array_slice($sub_array, 0, 0) );
  var_dump ( array_slice($sub_array, 0, -3) );
  var_dump ( array_slice($sub_array, -2, 3) );
  var_dump ( array_slice($sub_array, -2, 0 ) );
  var_dump ( array_slice($sub_array, -2, -3) );

  /* variations with four arguments */
  /* offset value, length value and preserve_key values variation */
  echo"\n*** Variation with first two arguments with preserve_key value TRUE ***\n";
  var_dump ( array_slice($sub_array, 1, 3, true) );
  var_dump ( array_slice($sub_array, 1, 0, true) );
  var_dump ( array_slice($sub_array, 1, -3, true) );
  var_dump ( array_slice($sub_array, 0, 3, true) );
  var_dump ( array_slice($sub_array, 0, 0, true) );
  var_dump ( array_slice($sub_array, 0, -3, true) );
  var_dump ( array_slice($sub_array, -2, 3, true) );
  var_dump ( array_slice($sub_array, -2, 0, true) );
  var_dump ( array_slice($sub_array, -2, -3, true) );
  $counter++;
}

  /* variation of offset and length to point to same element */
  echo"\n*** Typical Variation of offset and length  Arguments ***\n";
  var_dump (array_slice($var_array[2], 1, -3, true) );
  var_dump (array_slice($var_array[2], 1, -3, false) );
  var_dump (array_slice($var_array[2], -3, -2, true) );
  var_dump (array_slice($var_array[2], -3, -2, false) );

?>