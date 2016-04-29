<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);
ldap_add($link, "cn=userC,$base", array(
	"objectclass" => "person",
	"cn" => "userC",
	"sn" => "zzz",
	"userPassword" => "oops",
	"description" => "a user",
));
ldap_add($link, "cn=userD,$base", array(
	"objectclass" => "person",
	"cn" => "userD",
	"sn" => "aaa",
	"userPassword" => "oops",
	"description" => "another user",
));
ldap_add($link, "cn=userE,$base", array(
	"objectclass" => "person",
	"cn" => "userE",
	"sn" => "a",
	"userPassword" => "oops",
	"description" => "yet another user",
));
$result = ldap_search($link, "$base", "(objectclass=person)", array("sn", "description"));
var_dump(
	ldap_sort($link, $result, "description"),
	ldap_get_entries($link, $result)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
ldap_delete($link, "cn=userC,$base");
ldap_delete($link, "cn=userD,$base");
ldap_delete($link, "cn=userE,$base");
remove_dummy_data($link, $base);
?>
