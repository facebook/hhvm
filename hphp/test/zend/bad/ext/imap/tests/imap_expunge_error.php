<?php
echo "Checking with no parameters\n";
imap_num_recent();

echo  "Checking with incorrect parameter type\n";
imap_num_recent('');
imap_num_recent(false);
?>