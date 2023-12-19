<?hh

/*
Default values are "localhost", "cn=Manager,dc=my-domain,dc=com", and password "secret".
Change the LDAP_TEST_* environment values if you want to use another configuration.
*/

function test_host(): string {
  return getenv("LDAP_TEST_HOST") ?: "localhost";
}
function test_port(): int {
  return intval(getenv("LDAP_TEST_PORT") ?: "389");
}
function test_base(): string {
  return getenv("LDAP_TEST_BASE") ?: "dc=my-domain,dc=com";
}
function test_user(): string {
  $base = test_base();
  return getenv("LDAP_TEST_USER") ?: "cn=Manager,$base";
}
function test_sasl_user(): string {
  return getenv("LDAP_TEST_SASL_USER") ?: "Manager";
}
function test_passwd(): string {
  return getenv("LDAP_TEST_PASSWD") ?: "secret";
}
function test_protocol_version(): int {
  return intval(getenv("LDAP_TEST_OPT_PROTOCOL_VERSION") ?: "3");
}
function test_skip_on_bind_failure(): bool {
  return boolval(getenv("LDAP_TEST_SKIP_BIND_FAILURE") ?: "1");
}

function ldap_connect_and_bind(
  $host, $port, $user, $passwd, $protocol_version,
) {
  $link = ldap_connect($host, $port);
  ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);
  ldap_bind($link, $user, $passwd);
  return $link;
}

function insert_dummy_data($link, $base) {
  // Create root if not there
  $testBase = ldap_read($link, $base, '(objectClass=*)', vec['objectClass']);
  if (ldap_count_entries($link, $testBase) < 1) {
    ldap_add(
      $link, "$base", dict[
        "objectClass" => vec[
          "top",
          "organization",
          "dcObject"
        ],
        "o" => "php ldap tests"
      ]
    );
  }
  ldap_add($link, "o=test,$base", dict[
    "objectClass" => vec[
      "top",
      "organization"],
    "o" => "test",
  ]);
  ldap_add($link, "cn=userA,$base", dict[
    "objectclass" => "person",
    "cn" => "userA",
    "sn" => "testSN1",
    "userPassword" => "oops",
    "telephoneNumber" => "xx-xx-xx-xx-xx",
    "description" => "user A",
  ]);
  ldap_add($link, "cn=userB,$base", dict[
    "objectclass" => "person",
    "cn" => "userB",
    "sn" => "testSN2",
    "userPassword" => "oopsIDitItAgain",
    "description" => "user B",
  ]);
  ldap_add($link, "cn=userC,cn=userB,$base", dict[
    "objectclass" => "person",
    "cn" => "userC",
    "sn" => "testSN3",
    "userPassword" => "0r1g1na1 passw0rd",
  ]);
}

function remove_dummy_data($link, $base) {
  ldap_delete($link, "cn=userC,cn=userB,$base");
  ldap_delete($link, "cn=userA,$base");
  ldap_delete($link, "cn=userB,$base");
  ldap_delete($link, "o=test,$base");
}
