<?hh

/* Explode with attributes */
<<__EntryPoint>> function main(): void {
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
try { ldap_explode_dn("cn=bob,dc=example,dc=com"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Too many parameters */
try { ldap_explode_dn("cn=bob,dc=example,dc=com", 1, 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Bad DN value with attributes */
var_dump(ldap_explode_dn("bob,dc=example,dc=com", 0));

/* Bad DN value without attributes */
var_dump(ldap_explode_dn("bob,dc=example,dc=com", 1));

echo "Done\n";
}
