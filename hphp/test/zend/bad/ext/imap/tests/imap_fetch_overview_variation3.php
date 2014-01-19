<?php
/* Prototype  : array imap_fetch_overview(resource $stream_id, int $msg_no [, int $options])
 * Description: Read an overview of the information in the headers of the given message sequence 
 * Source code: ext/imap/php_imap.c
 */

/*
 * Test passing a range of values into the $options argument to imap_fetch_overview():
 * 1. values that equate to 1
 * 2. Minimum and maximum PHP values
 */

echo "*** Testing imap_fetch_overview() : usage variations ***\n";

require_once(dirname(__FILE__).'/imap_include.inc');

// Initialise required variables
$stream_id = setup_test_mailbox('', 1); // set up temporary mailbox with one simple message
$msg_no = 1;
$msg_uid = imap_uid($stream_id, $msg_no);

$options = array ('1',
                  true,
                  1.000000000000001, 
                  0.00001e5, 
                  PHP_INT_MAX, 
                  -PHP_INT_MAX
                 );

// iterate over each element of $options array 
$iterator = 1;
imap_check($stream_id);
foreach($options as $option) {
	echo "\nTesting with option value:";
	var_dump($option);
	$overview = imap_fetch_overview($stream_id, $msg_uid, $option);
	if ($overview) {
                echo "imap_fetch_overview() returns an object\n";
        }
	$iterator++;
}

?>
===DONE===
<?php
require_once(dirname(__FILE__).'/clean.inc');
?>