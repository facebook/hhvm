<?hh
<<__EntryPoint>> function main(): void {
$subject = 'foo=bar(baz)*';

var_dump(ldap_escape($subject, '', LDAP_ESCAPE_FILTER));
}
