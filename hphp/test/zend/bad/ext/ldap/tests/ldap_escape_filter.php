<?php

$subject = 'foo=bar(baz)*';

var_dump(ldap_escape($subject, null, LDAP_ESCAPE_FILTER));

?>
