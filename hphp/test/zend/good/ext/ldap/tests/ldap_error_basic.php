<?hh
require "connect.inc";
<<__EntryPoint>> function main(): void {
$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
@ldap_add($link, "badDN $base", darray[
    "objectClass"   => varray[
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
