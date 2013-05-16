<?php
echo "Checking with no parameters\n";
imap_mail_copy();


echo  "Checking with incorrect parameter type\n";
imap_mail_copy('');
imap_mail_copy(false);


// more tests
require_once(dirname(__FILE__).'/imap_include.inc');


echo "Test with IMAP server\n";
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());
	
var_dump(imap_mail_copy($stream_id));
var_dump(imap_mail_copy($stream_id,-1));
var_dump(imap_mail_copy($stream_id, ''));

imap_close($stream_id);
?>
===Done===