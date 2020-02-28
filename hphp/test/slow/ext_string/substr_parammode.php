<?hh

<<__EntryPoint>>
function main_substr_parammode() {
try { var_dump(substr(varray[], 0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
