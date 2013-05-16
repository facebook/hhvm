<?php
/* Prototype  : string imap_fetchbody(resource $stream_id, int $msg_no, string $section 
 *           [, int $options])
 * Description: Get a specific body section 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different data types as $section argument to test behaviour of imap_fetchbody()
 */

echo "*** Testing imap_fetchbody() : usage variations ***\n";

require_once(dirname(__FILE__).'/imap_include.inc');

// Initialise function arguments not being substituted
$stream_id = setup_test_mailbox('', 1); // set up temp mailbox with 1 simple msg
$msg_no = 1;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $section argument
$inputs = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/*16*/ "",
       '',
       array(),

       // string data
/*19*/ "string",
       'string',
       $heredoc,
       
       // object data
/*22*/ new classA(),

       // undefined data
/*23*/ @$undefined_var,

       // unset data
/*24*/ @$unset_var,
);

// loop through each element of $inputs to check the behavior of imap_fetchbody()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( imap_fetchbody($stream_id, $msg_no, $input) );
  $iterator++;
};
?>
===DONE===<?php
require_once(dirname(__FILE__).'/clean.inc');
?>