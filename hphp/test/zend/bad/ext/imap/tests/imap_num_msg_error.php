<?php
echo "Checking with no parameters\n";
imap_num_msg();

echo  "Checking with incorrect parameter type\n";
imap_num_msg('');
imap_num_msg(false);
?>