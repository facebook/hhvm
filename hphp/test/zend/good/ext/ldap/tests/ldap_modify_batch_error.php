<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
$addGivenName = vec[
    dict[
        "attrib"    => "givenName",
        "modtype"   => LDAP_MODIFY_BATCH_ADD,
        "values"    => vec["Jack"]
    ]
];

// Too few parameters
try{
  var_dump(ldap_modify_batch());
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try{
  var_dump(ldap_modify_batch($link));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try{
  var_dump(ldap_modify_batch($link, "$base"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try{
  var_dump(ldap_modify_batch($link, "$base", $addGivenName, "Invalid additional parameter"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// DN not found
var_dump(ldap_modify_batch($link, "cn=not-found,$base", $addGivenName));

// Invalid DN
var_dump(ldap_modify_batch($link, "weirdAttribute=val", $addGivenName));

// prepare
$entry = dict[
    "objectClass"   => vec[
        "top",
        "dcObject",
        "organization"],
    "dc"            => "my-domain",
    "o"             => "my-domain",
];

ldap_add($link, "dc=my-domain,$base", $entry);

// invalid domain
$mods = vec[
    dict[
        "attrib"    => "dc",
        "modtype"   => LDAP_MODIFY_BATCH_REPLACE,
        "values"    => vec["Wrong Domain"]
    ]
];

var_dump(ldap_modify_batch($link, "dc=my-domain,$base", $mods));

// invalid attribute
$mods = vec[
    dict[
        "attrib"    => "weirdAttribute",
        "modtype"   => LDAP_MODIFY_BATCH_ADD,
        "values"    => vec["weirdVal", "anotherWeirdval"]
    ]
];

var_dump(ldap_modify_batch($link, "dc=my-domain,$base", $mods));
echo "===DONE===\n";
//--ldap_delete($link, "dc=my-domain,$base");
}
