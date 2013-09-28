<?php
echo "Checking with no parameters\n";
imap_expunge();

echo  "Checking with incorrect parameter type\n";
imap_expunge('');
imap_expunge(false);
?>