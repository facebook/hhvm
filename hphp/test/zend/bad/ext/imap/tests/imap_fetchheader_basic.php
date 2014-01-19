<?php
/* Prototype  : string imap_fetchheader(resource $stream_id, int $msg_no [, int $options])
 * Description: Get the full unfiltered header for a message 
 * Source code: ext/imap/php_imap.c
 */

echo "*** Testing imap_fetchheader() : basic functionality ***\n";
require_once(dirname(__FILE__).'/imap_include.inc');

// Initialise all required variables
$stream_id = setup_test_mailbox('', 1, $mailbox, 'multiPart'); // setup temp mailbox with 1 msg
$msg_no = 1;
$options = array('FT_UID' => FT_UID, 'FT_INTERNAL' => FT_INTERNAL, 
                 'FT_PREFETCHTEXT' => FT_PREFETCHTEXT);

// Calling imap_fetchheader() with all possible arguments
echo "\n-- All possible arguments --\n";
foreach ($options as $key => $option) {
	echo "-- Option is $key --\n";
	if ($key == 'FT_UID') {
		$msg_uid = imap_uid($stream_id, $msg_no);
		var_dump(imap_fetchheader($stream_id, $msg_uid, $option));
	} else {
		var_dump(imap_fetchheader($stream_id, $msg_no, $option));
	}
}

// Calling imap_fetchheader() with mandatory arguments
echo "\n-- Mandatory arguments --\n";
var_dump( imap_fetchheader($stream_id, $msg_no) );
?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>