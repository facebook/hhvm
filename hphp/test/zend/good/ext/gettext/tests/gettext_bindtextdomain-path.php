<?hh <<__EntryPoint>> function main_entry() {
chdir(dirname(__FILE__));
var_dump(bindtextdomain('example.org', 'foobar'));
}
