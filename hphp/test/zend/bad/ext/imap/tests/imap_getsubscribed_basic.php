<?php
echo "Checking with no parameters\n";
imap_getsubscribed();

echo  "Checking with incorrect parameter type\n";
imap_getsubscribed('');
imap_getsubscribed(false);

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());

imap_getsubscribed($stream_id);
imap_getsubscribed($stream_id,$default_mailbox);
var_dump(imap_getsubscribed($stream_id,$default_mailbox,'ezDvfXvbvcxSerz'));


echo "Checking OK\n";

$newbox = $default_mailbox . "." . $mailbox_prefix;

imap_createmailbox($stream_id, $newbox);
imap_subscribe($stream_id, $newbox);

$z = imap_getsubscribed($stream_id,$default_mailbox,'*');

var_dump(is_array($z));
var_dump($z[0]);

imap_close($stream_id);
?>
<?php 
require_once('clean.inc');
?>