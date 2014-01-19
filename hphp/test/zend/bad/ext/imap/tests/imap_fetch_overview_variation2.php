<?php
/* Prototype  : array imap_fetch_overview(resource $stream_id, int $msg_no [, int $options])
 * Description: Read an overview of the information in the headers 
 * of the given message sequence 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different data types as $msg_no argument to imap_fetch_overview() to test behaviour
 */

echo "*** Testing imap_fetch_overview() : usage variations ***\n";
require_once(dirname(__FILE__).'/imap_include.inc');

// Initialise function arguments not being substituted
$stream_id = setup_test_mailbox('', 1, $mailbox, 'notSimple'); // set up temp mailbox with 1 msg

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

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to <<<ARGUMENT HERE>>> argument
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

       // resource variable
/*25*/ $fp
);

// loop through each element of $inputs to check the behavior of imap_fetch_overview()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Testing with second argument value: ";
	var_dump($input);
	$overview = imap_fetch_overview($stream_id, $input);
	if (!$overview) {
		echo imap_last_error() . "\n";
	} else {
		displayOverviewFields($overview[0]);
        }
	$iterator++;
};

fclose($fp);

// clear the error stack
imap_errors();
?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>