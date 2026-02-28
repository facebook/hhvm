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

  ldap_set_option($link, LDAP_OPT_DEREF, LDAP_DEREF_NEVER);
  ldap_set_option($link, LDAP_OPT_SIZELIMIT, 123);
  ldap_set_option($link, LDAP_OPT_TIMELIMIT, 33);
  ldap_set_option($link, LDAP_OPT_NETWORK_TIMEOUT, 44);
  ldap_set_option($link, LDAP_OPT_REFERRALS, false);
  ldap_set_option($link, LDAP_OPT_SERVER_CONTROLS, $controls);
  ldap_set_option($link, LDAP_OPT_CLIENT_CONTROLS, $controls);
  ldap_set_option($link, LDAP_OPT_RESTART, false);

  var_dump(
  	ldap_get_option($link, LDAP_OPT_DEREF, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_SIZELIMIT, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_TIMELIMIT, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_NETWORK_TIMEOUT, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_REFERRALS, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_RESTART, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_SERVER_CONTROLS, inout $option),
  	$option,
  	ldap_get_option($link, LDAP_OPT_CLIENT_CONTROLS, inout $option),
  	$option
  );
  echo "===DONE===\n";
}
