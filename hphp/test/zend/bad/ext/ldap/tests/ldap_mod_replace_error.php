<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

// Too few parameters
var_dump(ldap_mod_replace());
var_dump(ldap_mod_replace($link));
var_dump(ldap_mod_replace($link, "dc=my-domain,dc=com"));

// Too many parameters
var_dump(ldap_mod_replace($link, "dc=my-domain,dc=com", array(), "Additional data"));

// DN not found
var_dump(ldap_mod_replace($link, "dc=my-domain,dc=com", array()));

// Invalid DN
var_dump(ldap_mod_replace($link, "weirdAttribute=val", array()));

// Invalid attributes
var_dump(ldap_mod_replace($link, "dc=my-domain,dc=com", array('dc')));
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

ldap_delete($link, "dc=my-domain,dc=com");
?>