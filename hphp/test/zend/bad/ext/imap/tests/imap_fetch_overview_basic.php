<?php
/* Prototype  : array imap_fetch_overview(resource $stream_id, int $msg_no [, int $options])
 * Description: Read an overview of the information in the headers 
 * of the given message sequence 
 * Source code: ext/imap/php_imap.c
 */

echo "*** Testing imap_fetch_overview() : basic functionality ***\n";

require_once(dirname(__FILE__).'/imap_include.inc');

// create a new mailbox and add two new messages to it
$stream_id = setup_test_mailbox('', 2, $mailbox, 'notSimple');

// get UID for new message
$msg_no = imap_uid($stream_id, 1);
$options = FT_UID;

// Calling imap_fetch_overview() with all possible arguments
echo "\n-- All possible arguments --\n";
$a =  imap_fetch_overview($stream_id, "$msg_no", $options) ;
echo "\n--> Object #1\n";
displayOverviewFields($a[0]);

// Calling imap_fetch_overview() with mandatory arguments
echo "\n-- Mandatory arguments --\n";
$a = imap_fetch_overview($stream_id, '1:2') ;

//first object in array
echo "\n--> Object #1\n";
displayOverviewFields($a[0]);

//Second object in array
echo "\n--> Object #2\n";
displayOverviewFields($a[1]);

imap_close($stream_id);

?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>