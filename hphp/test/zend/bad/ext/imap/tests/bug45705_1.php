<?php

$address = 'John Doe <john@example.com>';
var_dump($address);
imap_rfc822_parse_adrlist($address, null);
var_dump($address);

?>