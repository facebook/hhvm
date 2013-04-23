<?php

var_dump(imap_mutf7_to_utf8(""));
var_dump(imap_mutf7_to_utf8(1));
var_dump(imap_mutf7_to_utf8(array(1,2)));
var_dump(imap_mutf7_to_utf8("t&AOQ-st"));

echo "Done\n";
?>