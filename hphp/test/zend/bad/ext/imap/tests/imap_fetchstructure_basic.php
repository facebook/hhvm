<?php
echo "Checking with no parameters\n";
imap_fetchstructure();

echo  "Checking with incorrect parameter type\n";
imap_fetchstructure('');
imap_fetchstructure(false);

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = setup_test_mailbox('', 1);

imap_fetchstructure($stream_id);
imap_fetchstructure($stream_id,0);

$z = imap_fetchstructure($stream_id,1);


$fields = array('type','encoding','ifsubtype','subtype',
'ifdescription','lines','bytes','parameters');

foreach ($fields as $key) {
	var_dump(isset($z->$key));
}
var_dump($z->type);
var_dump($z->encoding);
var_dump($z->bytes);
var_dump($z->lines);
var_dump(is_object($z->parameters));

imap_close($stream_id);
?>
<?php 
require_once('clean.inc');
?>