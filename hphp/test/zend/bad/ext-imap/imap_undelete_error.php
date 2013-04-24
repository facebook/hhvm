<?php
echo "Checking with no parameters\n";
imap_undelete();

echo  "Checking with incorrect parameter type\n";
imap_undelete('');
imap_undelete(false);

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());
	
imap_undelete($stream_id);

imap_close($stream_id);
?>