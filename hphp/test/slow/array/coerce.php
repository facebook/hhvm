<?hh


<<__EntryPoint>>
function main_coerce() {
try { var_dump(Locale::lookup(new stdclass, 'foo')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(Locale::lookup(STDIN, 'foo')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
