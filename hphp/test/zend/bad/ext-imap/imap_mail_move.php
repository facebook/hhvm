<?php
echo "Checking with no parameters\n";
imap_mail_move();


echo  "Checking with incorrect parameter type\n";
imap_mail_move('');
imap_mail_move(false);


// more tests
require_once(dirname(__FILE__).'/imap_include.inc');


echo "Test with IMAP server\n";
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());
	
var_dump(imap_mail_move($stream_id));
var_dump(imap_mail_move($stream_id,-1));
var_dump(imap_mail_move($stream_id, ''));

imap_close($stream_id);
?>
===Done===