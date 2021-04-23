<?hh

function skip_if_bind_failure(): void {
  if (!test_skip_on_bind_failure()) return;

  $link = ldap_connect(test_host(), test_port());
  ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());
  if (!@ldap_bind($link, test_user(), test_passwd())) {
    die(sprintf(
      "skip Can't bind to LDAP Server - [%d] %s",
      ldap_errno($link),
      ldap_error($link),
    ));
  }
  @ldap_unbind($link);
}
