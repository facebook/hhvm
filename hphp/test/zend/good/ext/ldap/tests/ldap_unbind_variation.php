<?hh

require "connect.inc";

function rebind_proc ($ds, $ldap_url) {
	// required by most modern LDAP servers, use LDAPv3
	ldap_set_option($a, LDAP_OPT_PROTOCOL_VERSION, ZendGoodExtLdapTestsLdapUnbindVariation::$protocol_version);

	if (!ldap_bind($a, ZendGoodExtLdapTestsLdapUnbindVariation::$user, ZendGoodExtLdapTestsLdapUnbindVariation::$passwd)) {
		print "Cannot bind";
	}
}

abstract final class ZendGoodExtLdapTestsLdapUnbindVariation {
  public static $user;
  public static $passwd;
  public static $protocol_version;
}

<<__EntryPoint>>
function main_entry(): void {
  $link = ldap_connect_and_bind($host, $port, ZendGoodExtLdapTestsLdapUnbindVariation::$user, ZendGoodExtLdapTestsLdapUnbindVariation::$passwd, ZendGoodExtLdapTestsLdapUnbindVariation::$protocol_version);
  ldap_set_rebind_proc($link, "rebind_proc");

  var_dump(ldap_unbind($link));

  echo "===DONE===\n";
}
