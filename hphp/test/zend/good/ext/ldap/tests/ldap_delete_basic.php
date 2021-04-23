<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
ldap_add($link, "dc=my-domain,$base", dict[
    "objectClass"   => vec[
        "top",
        "dcObject",
        "organization"],
    "dc"            => "my-domain",
    "o"             => "my-domain",
]);

var_dump(
    ldap_delete($link, "dc=my-domain,$base"),
    @ldap_search($link, "dc=my-domain,$base", "(o=my-domain)")
);
echo "===DONE===\n";
//--@ldap_delete($link, "dc=my-domain,$base");
}
