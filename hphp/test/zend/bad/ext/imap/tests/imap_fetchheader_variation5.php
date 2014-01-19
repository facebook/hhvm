<?php
/* Prototype  : string imap_fetchheader(resource $stream_id, int $msg_no [, int $options])
 * Description: Get the full unfiltered header for a message 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different integers and strings as $msg_no argument 
 * to test behaviour of imap_fetchheader()
 */

echo "*** Testing imap_fetchheader() : usage variations ***\n";

require_once(dirname(__FILE__).'/imap_include.inc');

$stream_id = setup_test_mailbox('', 3, $mailbox, 'notSimple'); // set up temp mailbox with 3 msgs

$sequences = array (0,     4, // out of range
                    '1,3', '1:3', // message sequences instead of numbers
                    ); 

foreach($sequences as $msg_no) {
	echo "\n-- \$msg_no is $msg_no --\n";
	var_dump($overview = imap_fetchheader($stream_id, $msg_no));
	if (!$overview) {
		echo imap_last_error() . "\n";
	}
}

// clear error stack
imap_errors();
?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>