<?php

var_dump(imap_utf8_to_mutf7(""));
var_dump(imap_utf8_to_mutf7(1));
var_dump(imap_utf8_to_mutf7(array(1,2)));
var_dump(imap_utf8_to_mutf7("täst"));

echo "Done\n";
?>