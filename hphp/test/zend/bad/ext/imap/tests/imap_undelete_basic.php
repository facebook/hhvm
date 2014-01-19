<?php

require_once(dirname(__FILE__).'/imap_include.inc');
$stream_id = setup_test_mailbox('', 1);

imap_delete($stream_id, 1);

var_dump(imap_undelete($stream_id, 1));

imap_close($stream_id);

?>
<?php 
require_once('clean.inc');
?>