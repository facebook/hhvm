<?php

var_dump(imap_utf8(""));
var_dump(imap_utf8(1));
var_dump(imap_utf8(array(1,2)));
var_dump(imap_utf8("test"));

echo "Done\n";
?>