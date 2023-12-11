<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
@ldap_add($link, "badDN $base", dict[
    "objectClass"   => vec[
        "top",
        "dcObject",
        "organization"],
    "dc"            => "my-domain",
    "o"             => "my-domain",
]);

var_dump(
    ldap_error($link)
);
echo "===DONE===\n";
}
