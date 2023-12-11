<?hh
<<__EntryPoint>>
function main_entry(): void {
  require "connect.inc";

  $link = ldap_connect(test_host(), test_port());
  $option = null;

  $controls = vec[
  	dict["oid" => "1.2.752.58.10.1", "iscritical" => true],
  	dict["oid" => "1.2.752.58.1.10", "value" => "magic"],
  ];

  var_dump(ldap_set_option($link, LDAP_OPT_DEREF, LDAP_DEREF_ALWAYS));
  ldap_get_option($link, LDAP_OPT_DEREF, inout $option);
  var_dump(
  	$option === LDAP_DEREF_ALWAYS,
  	ldap_set_option($link, LDAP_OPT_SIZELIMIT, 123)
  );
  ldap_get_option($link, LDAP_OPT_SIZELIMIT, inout $option);
  var_dump(
  	$option,
  	ldap_set_option($link, LDAP_OPT_TIMELIMIT, 33)
  );
  ldap_get_option($link, LDAP_OPT_TIMELIMIT, inout $option);
  var_dump(
  	$option,
  	ldap_set_option($link, LDAP_OPT_NETWORK_TIMEOUT, 44)
  );
  ldap_get_option($link, LDAP_OPT_NETWORK_TIMEOUT, inout $option);
  var_dump(
  	$option,
  	ldap_set_option($link, LDAP_OPT_REFERRALS, true)
  );
  ldap_get_option($link, LDAP_OPT_REFERRALS, inout $option);
  var_dump(
  	(bool) $option,
  	ldap_set_option($link, LDAP_OPT_RESTART, false)
  );
  ldap_get_option($link, LDAP_OPT_RESTART, inout $option);
  var_dump(
  	(bool) $option,
  	ldap_set_option($link, LDAP_OPT_SERVER_CONTROLS, $controls)
  );
  ldap_get_option($link, LDAP_OPT_SERVER_CONTROLS, inout $option);
  var_dump(
  	$option,
  	ldap_set_option($link, LDAP_OPT_CLIENT_CONTROLS, $controls)
  );
  ldap_get_option($link, LDAP_OPT_CLIENT_CONTROLS, inout $option);
  var_dump(
  	$option,
  	ldap_set_option($link, LDAP_OPT_MATCHED_DN, "dc=test,dc=com")
  );
  ldap_get_option($link, LDAP_OPT_MATCHED_DN, inout $option);
  var_dump($option);
  echo "===DONE===\n";
}
