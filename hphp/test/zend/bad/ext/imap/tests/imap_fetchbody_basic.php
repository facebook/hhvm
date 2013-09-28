<?php
/* Prototype  : string imap_fetchbody(resource $stream_id, int $msg_no, string $section 
 *           [, int $options])
 * Description: Get a specific body section 
 * Source code: ext/imap/php_imap.c
 */

echo "*** Testing imap_fetchbody() : basic functionality ***\n";
require_once(dirname(__FILE__).'/imap_include.inc');

// Initialise all required variables

// set up mailbox with one message
$stream_id = setup_test_mailbox('', 1, $mailbox, 'notSimple');

$msg_no = 1;
$section = '2';
$options = array ('FT_UID' => FT_UID, 'FT_PEEK' => FT_PEEK, 'FT_INTERNAL' => FT_INTERNAL);

// Calling imap_fetchbody() with all possible arguments
echo "\n-- All possible arguments --\n";
foreach ($options as $key => $option) {
	echo "-- Option is $key --\n";
	switch ($key) {

		case 'FT_UID';
		$msg_uid = imap_uid($stream_id, $msg_no);
		var_dump( imap_fetchbody($stream_id, $msg_uid, $section, $option) );
		break;
		
		case 'FT_PEEK';
		var_dump( imap_fetchbody($stream_id, $msg_no, $section, $option) );
		$overview = imap_fetch_overview($stream_id, 1);
		echo "Seen Flag: ";
		var_dump( $overview[0]->seen );
		break;
		
		case 'FT_INTERNAL';
		var_dump( imap_fetchbody($stream_id, $msg_no, $section, $option) );
		break;

	}
}

// Calling imap_fetchbody() with mandatory arguments
echo "\n-- Mandatory arguments --\n";
var_dump( imap_fetchbody($stream_id, $msg_no, $section) );
$overview = imap_fetch_overview($stream_id, 1);
echo "Seen Flag: ";
var_dump( $overview[0]->seen );
?>
===DONE===?>
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>