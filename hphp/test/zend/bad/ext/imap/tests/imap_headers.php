<?php
echo "Checking with no parameters\n";
imap_headers();

echo  "Checking with incorrect parameter type\n";
imap_headers('');
imap_headers(false);
?>