<?php
echo "Checking with no parameters\n";
imap_renamemailbox();

echo  "Checking with incorrect parameter type\n";
imap_renamemailbox('');
imap_renamemailbox(false);


require_once(dirname(__FILE__).'/imap_include.inc');
	
$stream_id = setup_test_mailbox('', 1);

if (!is_resource($stream_id)) {
	exit("TEST FAILED: Unable to create test mailbox\n");
}

$newbox = $default_mailbox . "." . $mailbox_prefix;

imap_renamemailbox($stream_id, $newbox.'not');
imap_renamemailbox($stream_id, $newbox);

//commented because of bug #49901
//$ancError = error_reporting(0);
//$z = imap_renamemailbox($stream_id, $newbox.'not2', $newbox.'2');
//var_dump($z);
//error_reporting($ancError);
echo "Checking OK\n";


var_dump(imap_createmailbox($stream_id, $newbox.'.test'));
var_dump(imap_renamemailbox($stream_id, $newbox.'.test', $newbox.'.testd'));

imap_close($stream_id);
?>
<?php 
require_once('clean.inc');
?>