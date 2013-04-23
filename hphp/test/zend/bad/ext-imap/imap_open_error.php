<?php
echo "Checking with no parameters\n";
imap_open();
imap_open(false);
imap_open(false, false);
imap_open('');
imap_open('', '');

echo "Checking with incorrect parameters\n" ;
imap_open('', '', '');
imap_open('', '', '', -1);

require_once(dirname(__FILE__).'/imap_include.inc');
imap_open($default_mailbox, $username, $password, NIL, -1);

?>