<?php
/* Prototype  : string mb_strto[lower|upper](string $sourcestring [, string $encoding])
 * Description: Returns a [lower|upper]cased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Two error messages returned for incorrect encoding for mb_strto[upper|lower]
 * Bug now appears to be fixed
 */

$sourcestring = 'Hello, World';

$inputs = array(12345, 12.3456789000E-10, true, false, "");
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( mb_strtolower($sourcestring, $input) );
  var_dump( mb_strtoupper($sourcestring, $input) );
  $iterator++;
};
?>