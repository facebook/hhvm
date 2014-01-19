<?php
require "connect.inc";

$link = ldap_connect($host, $port);
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);
var_dump(ldap_sasl_bind($link, null, $passwd, 'DIGEST-MD5', 'realm', $sasl_user));
?>
===DONE===