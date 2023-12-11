<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(trigger_error()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(trigger_error("error"));
try { var_dump(trigger_error(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(trigger_error("error", -1));
var_dump(trigger_error("error", 0));
var_dump(trigger_error("error", E_USER_WARNING));
var_dump(trigger_error("error", E_USER_DEPRECATED));

echo "Done\n";
}
