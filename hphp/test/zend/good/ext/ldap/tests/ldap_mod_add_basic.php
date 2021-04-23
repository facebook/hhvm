<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
$entry = dict[
    "description"   => "Domain description",
];

var_dump(
    ldap_mod_add($link, "o=test,$base", $entry),
    ldap_get_entries(
        $link,
        ldap_search($link, "o=test,$base", "(Description=Domain description)")
    )
);
echo "===DONE===\n";
//--remove_dummy_data($link, $base);
}
