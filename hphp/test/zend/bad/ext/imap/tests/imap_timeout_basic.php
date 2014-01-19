<?php
echo "Checking with no parameters\n";
imap_timeout();

echo  "Checking with incorrect parameter type\n";
imap_timeout('');
imap_timeout(false);

echo "GET values:\n";
var_dump(imap_timeout(IMAP_OPENTIMEOUT));
var_dump(imap_timeout(IMAP_READTIMEOUT));
var_dump(imap_timeout(IMAP_WRITETIMEOUT));
var_dump(imap_timeout(IMAP_CLOSETIMEOUT));

echo "SET values:\n";
var_dump(imap_timeout(IMAP_OPENTIMEOUT, 10));
var_dump(imap_timeout(IMAP_READTIMEOUT, 10));
var_dump(imap_timeout(IMAP_WRITETIMEOUT, 10));

//IMAP_CLOSETIMEOUT not implemented
//var_dump(imap_timeout(IMAP_CLOSETIMEOUT, 10));

echo "CHECK values:\n";
var_dump(imap_timeout(IMAP_OPENTIMEOUT));
var_dump(imap_timeout(IMAP_READTIMEOUT));
var_dump(imap_timeout(IMAP_WRITETIMEOUT));

//IMAP_CLOSETIMEOUT not implemented
//var_dump(imap_timeout(IMAP_CLOSETIMEOUT));

?>