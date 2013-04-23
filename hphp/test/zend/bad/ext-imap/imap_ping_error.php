<?php
echo "Checking with no parameters\n";
imap_ping();

echo  "Checking with incorrect parameter type\n";
imap_ping('');
imap_ping(false);
?>