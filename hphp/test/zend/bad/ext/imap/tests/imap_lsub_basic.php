<?php
echo "Checking with no parameters\n";
imap_lsub();

echo  "Checking with incorrect parameter type\n";
imap_lsub('');
imap_lsub(false);

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = imap_open($default_mailbox, $username, $password) or 
	die("Cannot connect to mailbox $default_mailbox: " . imap_last_error());

imap_lsub($stream_id);
imap_lsub($stream_id,$default_mailbox);
var_dump(imap_lsub($stream_id,$default_mailbox,'ezDvfXvbvcxSerz'));


echo "Checking OK\n";

$newbox = $default_mailbox . "." . $mailbox_prefix;

imap_createmailbox($stream_id, $newbox);
imap_subscribe($stream_id, $newbox);

$z = imap_lsub($stream_id,$default_mailbox,'*');

var_dump(is_array($z));
var_dump($z[0]);

imap_close($stream_id);
?>
<?php 
require_once('clean.inc');
?>