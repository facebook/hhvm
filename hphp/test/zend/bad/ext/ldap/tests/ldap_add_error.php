<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

// Too few parameters
var_dump(ldap_add());
var_dump(ldap_add($link));
var_dump(ldap_add($link, "dc=my-domain,dc=com"));

// Too many parameters
var_dump(ldap_add($link, "dc=my-domain,dc=com", array(), "Additional data"));

var_dump(ldap_add($link, "dc=my-domain,dc=com", array()));

// Invalid DN
var_dump(
	ldap_add($link, "weirdAttribute=val", array(
		"weirdAttribute"			=> "val",
	)),
	ldap_error($link),
	ldap_errno($link)
);

// Duplicate entry
for ($i = 0; $i < 2; $i++)
	var_dump(
		ldap_add($link, "dc=my-domain,dc=com", array(
			"objectClass"	=> array(
				"top",
				"dcObject",
				"organization"),
			"dc"			=> "my-domain",
			"o"				=> "my-domain",
		))
	);
var_dump(ldap_error($link), ldap_errno($link));

// Wrong array indexes
var_dump(
	ldap_add($link, "dc=my-domain2,dc=com", array(
		"objectClass"	=> array(
			0	=> "top",
			2	=> "dcObject",
			5	=> "organization"),
		"dc"			=> "my-domain",
		"o"				=> "my-domain",
	))
	/* Is this correct behaviour to still have "Already exists" as error/errno?
	,
	ldap_error($link),
	ldap_errno($link)
	*/
);

// Invalid attribute
var_dump(
	ldap_add($link, "dc=my-domain,dc=com", array(
		"objectClass"	=> array(
			"top",
			"dcObject",
			"organization"),
		"dc"			=> "my-domain",
		"o"				=> "my-domain",
		"weirdAttr"		=> "weirdVal",
	)),
	ldap_error($link),
	ldap_errno($link)
);

var_dump(
	ldap_add($link, "dc=my-domain,dc=com", array(array( "Oops"
	)))
	/* Is this correct behaviour to still have "Undefined attribute type" as error/errno?
	,
	ldap_error($link),
	ldap_errno($link)
	*/
);
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

ldap_delete($link, "dc=my-domain,dc=com");
?>