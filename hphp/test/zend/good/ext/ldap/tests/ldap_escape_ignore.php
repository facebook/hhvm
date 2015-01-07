<?php

$subject = 'foo=bar(baz)*';
$ignore = 'ao';

var_dump(ldap_escape($subject, $ignore));

?>
