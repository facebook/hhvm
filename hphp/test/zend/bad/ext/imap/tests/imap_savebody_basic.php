<?php
echo "Checking with no parameters\n";
imap_savebody();

echo  "Checking with incorrect parameter type\n";
imap_savebody('');
imap_savebody(false);

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = setup_test_mailbox('', 1);

imap_savebody($stream_id);

$file = dirname(__FILE__).'/tmpsavebody.txt';

//with URL
$z = imap_savebody($stream_id, $file, 1);
var_dump($z);
echo "Size: ".filesize($file)."\n";

//With FOPEN
$fp = fopen($file, 'w');
$z = imap_savebody($stream_id, $fp, 1);
fclose($fp);
var_dump($z);
echo "Size: ".filesize($file)."\n";

imap_close($stream_id);
?>
<?php 
@unlink(dirname(__FILE__).'/tmpsavebody.txt');
require_once('clean.inc');
?>