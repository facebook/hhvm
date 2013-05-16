<?php
// Too few parameters
var_dump(ldap_error());

// Too many parameters
var_dump(ldap_error(null, null));
?>
===DONE===