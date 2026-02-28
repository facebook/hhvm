<?hh <<__EntryPoint>> function main_entry() :mixed{
chdir(dirname(__FILE__));
var_dump(bindtextdomain('example.org', 'foobar'));
}
