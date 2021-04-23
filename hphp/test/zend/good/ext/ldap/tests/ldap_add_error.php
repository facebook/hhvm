<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
// Too few parameters
try {
  var_dump(ldap_add());
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_add($link));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_add($link, "$base"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try {
  var_dump(ldap_add($link, "$base", dict[], "Additional data"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(ldap_add($link, "$base", dict[]));

// Invalid DN
var_dump(
    ldap_add($link, "weirdAttribute=val", dict[
        "weirdAttribute"            => "val",
    ]),
    ldap_error($link),
    ldap_errno($link)
);

// Duplicate entry
for ($i = 0; $i < 2; $i++)
    var_dump(
    ldap_add($link, "dc=my-domain,$base", dict[
      "objectClass" => vec[
        "top",
        "dcObject",
        "organization"],
      "dc"          => "my-domain",
      "o"               => "my-domain",
    ])
    );
var_dump(ldap_error($link), ldap_errno($link));

// Wrong array indexes
var_dump(
    ldap_add($link, "dc=my-domain2,dc=com", dict[
        "objectClass"   => dict[
            0   => "top",
            2   => "dcObject",
            5   => "organization"],
        "dc"            => "my-domain",
        "o"             => "my-domain",
    ])
    /* Is this correct behaviour to still have "Already exists" as error/errno?
    ,
    ldap_error($link),
    ldap_errno($link)
    */
);

// Invalid attribute
var_dump(
    ldap_add($link, "$base", dict[
        "objectClass"   => vec[
            "top",
            "dcObject",
            "organization"],
        "dc"            => "my-domain",
        "o"             => "my-domain",
        "weirdAttr"     => "weirdVal",
    ]),
    ldap_error($link),
    ldap_errno($link)
);

var_dump(
    ldap_add($link, "$base", dict[0 => vec["Oops"]]),
    /* Is this correct behaviour to still have "Undefined attribute type" as error/errno?
    ldap_error($link),
    ldap_errno($link)
    */
);
echo "===DONE===\n";
//--ldap_delete($link, "dc=my-domain,$base");
}
