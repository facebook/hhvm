<?php
/* Prototype  : bool imap_append  ( resource $imap_stream  , string $mailbox  , string $message  [, string $options  ] )
 * Description: Append a string message to a specified mailbox.
 * Source code: ext/imap/php_imap.c
 */

echo "*** Testing imap_append() : basic functionality ***\n";

require_once(dirname(__FILE__).'/imap_include.inc');

echo "Create a new mailbox for test\n";
$imap_stream = setup_test_mailbox("", 0);
if (!is_resource($imap_stream)) {
	exit("TEST FAILED: Unable to create test mailbox\n");
}

$mb_details = imap_mailboxmsginfo($imap_stream);
echo "Add a couple of msgs to new mailbox " . $mb_details->Mailbox . "\n"; 
var_dump(imap_append($imap_stream, $mb_details->Mailbox
                   , "From: webmaster@something.com\r\n"
                   . "To: info@something.com\r\n"
                   . "Subject: Test message\r\n"
                   . "\r\n"
                   . "this is a test message, please ignore\r\n"
                   ));
                   
var_dump(imap_append($imap_stream, $mb_details->Mailbox
                   , "From: webmaster@something.com\r\n"
                   . "To: info@something.com\r\n"
                   . "Subject: Another test\r\n"
                   . "\r\n"
                   . "this is another test message, please ignore it too!!\r\n"
                   ));     
                   
$check = imap_check($imap_stream);
echo "Msg Count after append : ". $check->Nmsgs . "\n";

echo "List the msg headers\n";
var_dump(imap_headers($imap_stream));

imap_close($imap_stream);
?>
===Done===
<?php 
require_once('clean.inc');
?>