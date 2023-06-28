<?hh

function rebind_proc ($ds, $ldap_url) :mixed{
  // required by most modern LDAP servers, use LDAPv3
  ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());

  if (!ldap_bind($ds, test_user(), test_passwd())) {
        print "Cannot bind";
  }
}

<<__EntryPoint>>
function main_entry(): void {
  require "connect.inc";
  $link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
  var_dump(ldap_set_rebind_proc($link, rebind_proc<>));
  var_dump(ldap_set_rebind_proc($link, ""));

  echo "===DONE===\n";
}
