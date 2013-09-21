<?php
echo "Checking with no parameters\n";
imap_body();

echo  "Checking with incorrect parameter type\n";
imap_body('');
imap_body(false);
require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());
imap_body($stream_id);
imap_body($stream_id,-1);
imap_body($stream_id,1,-1);

//Access not existing
var_dump(imap_body($stream_id, 999, FT_UID));

imap_close($stream_id);

?>