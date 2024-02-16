<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(convert_cyr_string()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(convert_cyr_string("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(convert_cyr_string("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(convert_cyr_string("", "", ""));

var_dump(convert_cyr_string((string)"[[[[[[", "q", "m"));
var_dump(convert_cyr_string((string)"[[[[[[", "k", "w"));
var_dump(convert_cyr_string((string)"[[[[[[", "m", "a"));
var_dump(convert_cyr_string((string)"[[[[[[", "d", "i"));
var_dump(convert_cyr_string((string)"[[[[[[", "w", "k"));
var_dump(convert_cyr_string((string)"[[[[[[", "i", "q"));
var_dump(convert_cyr_string((string)"", "d", "i"));

echo "Done\n";
}
