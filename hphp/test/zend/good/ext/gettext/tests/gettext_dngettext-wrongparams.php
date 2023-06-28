<?hh <<__EntryPoint>> function main_entry() :mixed{
chdir(dirname(__FILE__));
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('dngettextTest', './locale');

try { var_dump(dngettext('dngettextTest', 'item', 'items')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
