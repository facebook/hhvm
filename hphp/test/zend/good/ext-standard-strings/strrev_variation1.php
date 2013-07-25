<?php
/* Prototype  : string strrev(string $str);
 * Description: Reverse a string 
 * Source code: ext/standard/string.c
*/

/* Testing strrev() function with various double quoted strings for 'str' */

echo "*** Testing strrev() : with various double quoted strings ***\n";
$str = "Hiiii";

$strings = array(
  //strings containing escape chars
  "hello\\world",
  "hello\$world",
  "\ttesting\ttesting\tstrrev",
  "testing\rstrrev testing strrev",
  "testing\fstrrev \f testing \nstrrev",
  "\ntesting\nstrrev\n testing \n strrev",
  "using\vvertical\vtab",

  //polyndrome strings
  "HelloolleH",
  "ababababababa",

  //numeric + strings  
  "Hello123",
  "123Hello",
  "123.34Hello",
  "Hello1.234",
  "123Hello1.234",

  //concatenated strings
  "Hello".chr(0)."World",
  "Hello"."world",
  "Hello".$str,

  //strings containing white spaces
  "              h",
  "h              ",
  "      h        ",
  "h  e  l  l  o  ",

  //strings containing quotes
  "\hello\'world",
  "hello\"world",

  //special chars in strings 
  "t@@#$% %test ^test &test *test +test -test",
  "!test ~test `test` =test= @test@test.com",
  "/test/r\test\strrev\t\u /uu/",

  //only special chars
  "!@#$%^&*()_+=-`~"
);

$count = 1;
for( $index = 0; $index < count($strings); $index++ ) {
  echo "\n-- Iteration $count --\n";
  var_dump( strrev($strings[$index]) );
  $count ++;
}

echo "*** Done ***";
?>