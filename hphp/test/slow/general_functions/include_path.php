<?hh


<<__EntryPoint>>
function main_include_path() :mixed{

// originally came from ext/standard/tests/general_functions/include_path.phpt

var_dump(get_include_path());
try { var_dump(get_include_path("var")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(restore_include_path());
try { var_dump(restore_include_path("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


try { var_dump(set_include_path()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(get_include_path());
var_dump(set_include_path("var"));
var_dump(get_include_path());

var_dump(restore_include_path());
var_dump(get_include_path());

var_dump(set_include_path(".:/path/to/dir"));
var_dump(get_include_path());

var_dump(restore_include_path());
var_dump(get_include_path());

var_dump(set_include_path(""));
var_dump(get_include_path());

var_dump(restore_include_path());
var_dump(get_include_path());

var_dump(set_include_path('Array'));
var_dump(get_include_path());

var_dump(restore_include_path());
var_dump(get_include_path());


echo "Done\n";
}
