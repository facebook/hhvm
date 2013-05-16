<?php

/* Explode with attributes */
var_dump(ldap_explode_dn("cn=bob,dc=example,dc=com", 0));

/* Explode with attributes */
var_dump(ldap_explode_dn("cn=bob,ou=users,dc=example,dc=com", 0));

/* Explode without attributes */
var_dump(ldap_explode_dn("cn=bob,dc=example,dc=com", 1));

/* Explode without attributes */
var_dump(ldap_explode_dn("cn=bob,ou=users,dc=example,dc=com", 1));

/* Explode with attributes and < > characters */
var_dump(ldap_explode_dn("cn=<bob>,dc=example,dc=com", 0));

/* Explode without attributes and < > characters */
var_dump(ldap_explode_dn("cn=<bob>,dc=example,dc=com", 1));

/* Too few parameters */
ldap_explode_dn("cn=bob,dc=example,dc=com");

/* Too many parameters */
ldap_explode_dn("cn=bob,dc=example,dc=com", 1, 1);

/* Bad DN value with attributes */
var_dump(ldap_explode_dn("bob,dc=example,dc=com", 0));

/* Bad DN value without attributes */
var_dump(ldap_explode_dn("bob,dc=example,dc=com", 1));

echo "Done\n";

?>