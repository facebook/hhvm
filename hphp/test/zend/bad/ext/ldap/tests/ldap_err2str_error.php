<?php
// Too few args
var_dump(ldap_err2str());

// Too many args
var_dump(ldap_err2str(1, "Additional data"));

var_dump(ldap_err2str("weird"));
?>
===DONE===