<?hh <<__EntryPoint>> function main_entry() :mixed{
chdir(dirname(__FILE__));
try { bindtextdomain('foobar'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { bindtextdomain(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
