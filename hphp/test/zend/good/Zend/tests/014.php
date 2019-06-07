<?hh

var_dump(get_included_files());

include(dirname(__FILE__)."/014.inc");
var_dump(get_included_files());

try { var_dump(get_included_files(1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

include_once(dirname(__FILE__)."/014.inc");
var_dump(get_included_files());

try { var_dump(get_included_files(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

include(dirname(__FILE__)."/014.inc");
var_dump(get_included_files());

echo "Done\n";
