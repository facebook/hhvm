<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);
ldap_add($link, "cn=userC,dc=my-domain,dc=com", array(
	"objectclass" => "person",
	"cn" => "userC",
	"sn" => "zzz",
	"userPassword" => "oops",
	"description" => "a user",
));
ldap_add($link, "cn=userD,dc=my-domain,dc=com", array(
	"objectclass" => "person",
	"cn" => "userD",
	"sn" => "aaa",
	"userPassword" => "oops",
	"description" => "another user",
));
ldap_add($link, "cn=userE,dc=my-domain,dc=com", array(
	"objectclass" => "person",
	"cn" => "userE",
	"sn" => "a",
	"userPassword" => "oops",
	"description" => "yet another user",
));
$result = ldap_search($link, "dc=my-domain,dc=com", "(objectclass=person)", array("sn", "description"));
var_dump(
	ldap_sort($link, $result, "description"),
	ldap_get_entries($link, $result)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
ldap_delete($link, "cn=userC,dc=my-domain,dc=com");
ldap_delete($link, "cn=userD,dc=my-domain,dc=com");
ldap_delete($link, "cn=userE,dc=my-domain,dc=com");
remove_dummy_data($link);
?>