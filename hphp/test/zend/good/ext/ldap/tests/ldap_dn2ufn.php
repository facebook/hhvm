<?hh

/* Convert valid DN */
<<__EntryPoint>> function main(): void {
var_dump(ldap_dn2ufn("cn=bob,dc=example,dc=com"));
/* Convert valid DN */
var_dump(ldap_dn2ufn("cn=bob,ou=users,dc=example,dc=com"));

/* Convert DN with < > characters */
var_dump(ldap_dn2ufn("cn=<bob>,dc=example,dc=com"));

/* Too many parameters */
try { ldap_dn2ufn("cn=bob,dc=example,dc=com", 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Bad DN value */
var_dump(ldap_dn2ufn("bob,dc=example,dc=com"));

echo "Done\n";
}
