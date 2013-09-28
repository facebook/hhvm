<?php
/* Prototype  : bool imap_close(resource $stream_id [, int $options])
 * Description: Close an IMAP stream 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different data types as $options argument to test behaviour of imap_close()
 */

echo "*** Testing imap_close() : usage variations ***\n";

// include file for imap_stream
require_once(dirname(__FILE__).'/imap_include.inc');

// create mailbox to test whether options has been set to CL_EXPUNGE
$stream_id = setup_test_mailbox('', 3, $mailbox);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
32768
EOT;

// unexpected values to be passed to $options argument
$inputs = array(

       // int data
/*1*/  0,
       1,
       32768,
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
/*19*/ "32768",
       '32768',
       $heredoc,

       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,
);

// loop through each element of $inputs to check the behavior of imap_close()
$iterator = 1;
foreach($inputs as $input) {

	// mark added messages for deletion
	for ($i = 1; $i < 4; $i++) {
		imap_delete($stream_id, $i);
	}
	echo "\n-- Iteration $iterator --\n";
	var_dump( $check = imap_close($stream_id, $input) );

	// check that imap_close was successful, if not call imap_close and explicitly set CL_EXPUNGE
	if(false === $check) {
		imap_close($stream_id, CL_EXPUNGE);
	} else {
		// if imap_close was successful test whether CL_EXPUNGE was set by doing a message count
		$imap_stream = imap_open($mailbox, $username, $password);
		$num_msg = imap_num_msg($imap_stream);
		if ($num_msg != 0) {
			echo "CL_EXPUNGE was not set, $num_msg msgs in mailbox\n";
		} else {
			echo "CL_EXPUNGE was set\n";
		}
		// call imap_close with CL_EXPUNGE explicitly set in case mailbox not empty
		imap_close($imap_stream, CL_EXPUNGE);
	}
	$iterator++;

	// get $stream_id for next iteration
	$stream_id = imap_open($mailbox, $username, $password);
	populate_mailbox($stream_id, $mailbox, 3);

};

?>
===DONE===?>
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>