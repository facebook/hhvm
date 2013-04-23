<?php
// Too few parameters
var_dump(ldap_errno());

// Too many parameters
var_dump(ldap_errno(null, null));
?>
===DONE===