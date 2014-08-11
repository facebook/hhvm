<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with 'source' string containing invalid/unknown token value
 *  unknown tokens - T_UNKNOWN(307)
*/

echo "*** Testing token_get_all() : with invalid/unknown tokens ***\n";

// with valid php tags and invalid tokens
echo "-- with valid PHP tags & invlid tokens --\n";
$source = '<?php 
struct myStruct {
  variable $a;
  method() { display $a; }
}
?>';
var_dump( token_get_all($source));

// with invalid open tag for testing entire source to be unknown token
echo "-- with invlalid PHP open tag & valid tokens --\n";
$source = '<pli 
echo "hello world"; ?>';
var_dump( token_get_all($source));

// with invalid PHP tags and invalid tokens
echo "-- with invalid PHP tags and tokens --\n";
$source = '<PDP display  $a; <';
var_dump( token_get_all($source));

echo "Done"
?>
