<?hh <<__EntryPoint>> function main_entry() :mixed{
$base_dir = dirname(__FILE__);
chdir($base_dir);
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('messages','');
var_dump(gettext('Basic test'));
bindtextdomain('messages', './locale');
var_dump(gettext('Basic test'));
}
