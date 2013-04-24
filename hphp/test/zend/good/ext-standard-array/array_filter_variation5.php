<?php
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback. 
 * Source code: ext/standard/array.c
*/

/*
* With default callback function argument, array_filter() removes elements which are interpreted as false 
* Here Testing all the false array element possibilities
*/

// callback function always_true
function always_true($input)
{
  return true;
}

// callback function always_false
function always_false($input)
{
  return false;
}

echo "*** Testing array_filter() : usage variations - different false elements in 'input' ***\n";

// unset variable
$unset_var = 10;
unset($unset_var);

// empty heredoc string
$empty_heredoc =<<<EOT
EOT;

// input array with different false elements
$input = array(
  false,
  False,
  '',
  "",
  0,
  0.0,
  null,
  NULL,
  "0",
  '0',
  array(),
  !1,
  1==2,
  $empty_heredoc,
  @$unset_var,
  @$undefined_var,
);

// With default callback function
var_dump( array_filter($input) );  

// With callback function which returns always true
var_dump( array_filter($input, 'always_true') ); 

// With callback function which returns always false
var_dump( array_filter($input, 'always_false') );  

echo "Done"
?>