<?php
/* Prototype  : array imap_fetch_overview(resource $stream_id, int $msg_no [, int $options])
 * Description: Read an overview of the information in the headers 
 * of the given message sequence 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different sequences/msg numbers as $msg_no argument to test behaviour
 * of imap_fetch_overview()
 */

echo "*** Testing imap_fetch_overview() : usage variations ***\n";
require_once(dirname(__FILE__).'/imap_include.inc');

$stream_id = setup_test_mailbox('', 3, $mailbox, 'notSimple'); // set up temp mailbox with 3 msgs

$sequences = array (0,     4,     '4', // out of range
                    '2',   '1,3', '1, 2',
                    '1:3'); // pass uid without setting FT_UID option

foreach($sequences as $msg_no) {
	echo "\n-- \$msg_no is $msg_no --\n";
        $overview = imap_fetch_overview($stream_id, $msg_no);
	if (!$overview) {
		echo imap_last_error() . "\n";
        } else {
		foreach($overview as $ov) {
			echo "\n";
			displayOverviewFields($ov);
       		 }
        }
}

// clear error stack
imap_errors();
?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>