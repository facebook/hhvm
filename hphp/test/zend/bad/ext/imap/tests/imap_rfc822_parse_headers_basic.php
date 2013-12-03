<?php

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = setup_test_mailbox('', 1);

$z = imap_headerinfo($stream_id, 1);

$fields = array ('toaddress','to','fromaddress','from',
'reply_toaddress','reply_to',
	'senderaddress', 'sender', 
'subject','Subject',
	'MailDate','Size','udate');


echo "Check general fields\n";
foreach ($fields as $key) {
	var_dump(isset($z->$key));
}

echo "Check type\n";
var_dump($z->toaddress);
var_dump($z->fromaddress);
var_dump($z->reply_toaddress);
var_dump($z->senderaddress);
var_dump($z->subject);
var_dump($z->Subject);

if ($z->Recent == 'R' || $z->Recent == 'N' || $z->Recent == ' ') {
	echo "Recent: OK";
} else {
	echo "Recent: error";
}
echo "\n";

if ($z->Unseen == 'U' || $z->Unseen == ' ') {
	echo "Unseen: OK";
} else {
	echo "Unseen: error";
}
echo "\n";

if ($z->Flagged == 'F' || $z->Flagged == ' ') {
	echo "Flagged: OK";
} else {
	echo "Flagged: error";
}
echo "\n";

if ($z->Answered == 'A' || $z->Answered == ' ') {
	echo "Answered: OK";
} else {
	echo "Answered: error";
}
echo "\n";

if ($z->Deleted == 'D' || $z->Deleted == ' ') {
	echo "Deleted: OK";
} else {
	echo "Deleted: error";
}
echo "\n";

if ($z->Draft == 'X' || $z->Draft == ' ') {
	echo "Draft: OK";
} else {
	echo "Draft: error";
}
echo "\n";

var_dump($z->Msgno);
var_dump($z->Size);
var_dump($z->udate);

imap_close($stream_id);

?>
<?php 
require_once('clean.inc');
?>