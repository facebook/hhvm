<?php
echo "Checking with no parameters\n";
imap_gc();

echo  "Checking with incorrect parameter type\n";
imap_gc('', false);
imap_gc(false, false);

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());
imap_gc($stream_id, -1);

?>