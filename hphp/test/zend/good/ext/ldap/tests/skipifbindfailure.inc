<?hh

function skip_if_bind_failure(): void {
  if (!test_skip_on_bind_failure()) return;

  $link = ldap_connect(test_host(), test_port());
  ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());
  $error = error_reporting();
  error_reporting(0);
  if (!ldap_bind($link, test_user(), test_passwd())) {
    error_reporting($error);
    die(sprintf(
      "skip Can't bind to LDAP Server - [%d] %s",
      ldap_errno($link),
      ldap_error($link),
    ));
  }
  ldap_unbind($link);
  error_reporting($error);
}
