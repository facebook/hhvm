<?php

/* Various input arrays for different format types */

$float_variation  = array( "%f", "%-f", "%+f", "%7.2f", "%-7.2f", "%07.2f", "%-07.2f", "%'#7.2f" );
$float_numbers    = array( 0, 1, -1, 0.32, -0.32, 3.4. -3.4, 2.54, -2.54, 1.2345678e99, -1.2345678e99 );

$int_variation    = array( "%d", "%-d", "%+d", "%7.2d", "%-7.2d", "%07.2d", "%-07.2d", "%'#7.2d" );
$int_numbers      = array( 0, 1, -1, 2.7, -2.7, 23333333, -23333333, "1234" );

$char_variation   = array( 'a', "a", 67, -67, 99 );

$string_variation = array( "%5s", "%-5s", "%05s", "%'#5s" );
$strings          = array( NULL, "abc", 'aaa' );

/* Checking warning messages */

/* Zero argument */
echo "\n*** Output for zero argument ***\n";
printf();

/* Number of arguments not matching as specified in format field */
echo "\n*** Output for insufficient number of arguments ***\n";
$string = "dingy%sflem%dwombat";
$nbr = 5;
$name = "voudras";
printf("%d $string %s", $nbr, $name);


/* Scalar argument */
echo "\n*** Output for scalar argument ***\n";
printf(3);

/* NULL argument */
echo "\n*** Output for NULL as argument ***\n";
printf(NULL);


/* Float type variations */

$counter = 1;
echo "\n\n*** Output for float type ***\n";
echo "\n Input Float numbers variation array is:\n";
print_r($float_numbers);

foreach( $float_variation as $float_var )
{
 echo "\n\nFloat Iteration $counter";
 foreach( $float_numbers as $float_num )
 {
  echo "\n";
  printf( $float_var, $float_num );
 }
 $counter++;
}


/* Integer type variations */

$counter = 1;
echo "\n\n*** Output for integer type ***\n";
echo "\n Input Integer numbers variation array is:\n";
print_r($int_numbers);

foreach( $int_variation as $int_var )
{
 echo "\n\nInteger Iteration $counter";
 foreach( $int_numbers as $int_num )
 {
  echo "\n";
  printf( $int_var, $int_num );
 }
 $counter++;
}


/* Binary type variations */

echo "\n\n*** Output for binary type ***\n";
echo "\n Input  numbers variation array is:\n";
print_r($int_numbers);

 foreach( $int_numbers as $bin_num )
 {
  echo "\n";
  printf( "%b", $bin_num );
 }


/* Chararter type variations */
echo "\n\n*** Output for char type ***\n";
echo "\n Input Characters variation array is:\n";
print_r($char_variation);

foreach( $char_variation as $char )
{
 echo "\n";
 printf( "%c", $char );
}

/* Scientific type variations */
echo "\n\n*** Output for scientific type ***\n";
echo "\n Input numbers variation array is:\n";
print_r($int_numbers);

foreach( $int_numbers as $num )
{
 echo "\n";
 printf( "%e", $num );
}

/* Unsigned Integer type variation */
echo "\n\n*** Output for unsigned integer type ***\n";
echo "\n Input Integer numbers variation array is:\n";
print_r($int_numbers);

foreach( $int_numbers as $unsig_num )
{
 echo "\n";
 printf( "%u", $unsig_num );
}

/* Octal type variations */
echo "\n\n*** Output for octal type ***\n";
echo "\n Input numbers variation array is:\n";
print_r($int_numbers);

foreach( $int_numbers as $octal_num )
{
 echo "\n";
 printf( "%o", $octal_num );
}

/* Hexadecimal type variations */
echo "\n\n*** Output for hexadecimal type ***\n";
echo "\n Input numbers variation array is:\n";
print_r($int_numbers);

foreach( $int_numbers as $hexa_num )
{
 echo "\n";
 printf( "%x", $hexa_num );
}

/* String type variations */
echo "\n\n*** Output for string type ***\n";
echo "\n Input Strings format variation array is:\n";
print_r($string_variation);
echo "\n Input strings variation array is:\n";
print_r($strings);

foreach( $string_variation as $string_var )
{
 foreach( $strings as $str )
 {
  echo "\n";
  printf( $string_var, $str );
 }
}


/* variations of %g type */
$format_g = array("%g", "%.0g", "%+g", "%-g", "%-1.2g", "%+1.2g", "%G", "%.0G", "%+G", "%-G", "%-1.2G", "%+1.2G");

echo "\n\n*** Output for '%g' type ***\n";
echo "\n Input format variation array is:\n";
print_r($format_g);

foreach( $format_g as $formatg )
{
 printf("\n$formatg",123456);
 printf("\n$formatg",-123456);
}


/* Some more typical cases */

$tempnum = 12345;
$tempstring = "abcdefghjklmnpqrstuvwxyz";

echo"\n\n*** Output for '%%%.2f' as the format parameter ***\n";
printf("%%%.2f",1.23456789e10);

echo"\n\n*** Output for '%%' as the format parameter ***\n";
printf("%%",1.23456789e10);

echo"\n\n*** Output for precision value more than maximum ***\n";
printf("%.988f",1.23456789e10);   

echo"\n\n*** Output for invalid width(-15) specifier ***\n";
printf("%030.-15s", $tempstring);  

echo"\n\n*** Output for '%F' as the format parameter ***\n";
printf("%F",1.23456789e10);

echo"\n\n*** Output for '%X' as the format parameter ***\n";
printf("%X",12);

echo"\n\n*** Output  with no format parameter ***\n";
printf($tempnum);

echo"\n\n*** Output for multiple format parameters ***\n";
printf("%d  %s  %d\n", $tempnum, $tempstring, $tempnum); 

echo"\n\n*** Output for excess of mixed type arguments  ***\n";
printf("%s", $tempstring, $tempstring, $tempstring);      

echo"\n\n*** Output for string format parameter and integer type argument ***\n";
printf("%s", $tempnum);          

echo"\n\n*** Output for integer format parameter and string type argument ***\n";
printf("%d", $tempstring);    


?>