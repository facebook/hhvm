<?hh

require "connect.inc";

function rebind_proc ($ds, $ldap_url) {
  // required by most modern LDAP servers, use LDAPv3
  ldap_set_option($a, LDAP_OPT_PROTOCOL_VERSION, ZendGoodExtLdapTestsLdapSetRebindProcBasic::$protocol_version);

  if (!ldap_bind($a, ZendGoodExtLdapTestsLdapSetRebindProcBasic::$user, ZendGoodExtLdapTestsLdapSetRebindProcBasic::$passwd)) {
        print "Cannot bind";
  }
}

abstract final class ZendGoodExtLdapTestsLdapSetRebindProcBasic {
  public static $user;
  public static $passwd;
  public static $protocol_version;
}

<<__EntryPoint>>
function main_entry(): void {
  $link = ldap_connect_and_bind($host, $port, ZendGoodExtLdapTestsLdapSetRebindProcBasic::$user, ZendGoodExtLdapTestsLdapSetRebindProcBasic::$passwd, ZendGoodExtLdapTestsLdapSetRebindProcBasic::$protocol_version);
  var_dump(ldap_set_rebind_proc($link, "rebind_proc"));
  var_dump(ldap_set_rebind_proc($link, ""));

  echo "===DONE===\n";
}
