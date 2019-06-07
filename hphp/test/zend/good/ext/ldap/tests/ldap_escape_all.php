<?hh
<<__EntryPoint>> function main() {
$subject = 'foo=bar(baz)*';

var_dump(ldap_escape($subject));
}
