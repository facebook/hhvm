<?php
/* Prototype  : string imap_fetchbody(resource $stream_id, int $msg_no, string $section [, int $options])
 * Description: Get a specific body section 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different integers, strings, msg sequences and msg UIDs as $msg_no argument
 * to test behaviour of imap_fetchbody()
 */

echo "*** Testing imap_fetchbody() : usage variations ***\n";

require_once(dirname(__FILE__).'/imap_include.inc');

//Initialise required variables
$stream_id = setup_test_mailbox('', 3); // set up temp mailbox with  simple msgs
$section = 1;

$sequences = array (0,     4, // out of range
                    '1,3', '1:3', // message sequences instead of numbers
                   );

foreach($sequences as $msg_no) {
	echo "\n-- \$msg_no is $msg_no --\n";
	var_dump($overview = imap_fetchbody($stream_id, $msg_no, $section));
	if (!$overview) {
		echo imap_last_error() . "\n";
	}
}
?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>