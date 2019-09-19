<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(str_ireplace()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(str_ireplace("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(str_ireplace("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(str_ireplace("", "", ""));
$count = 0;
var_dump(str_ireplace("tt", "a", "ttttTttttttttTT"));
var_dump(str_ireplace_with_count("tt", "a", "ttttTttttttttTT", inout $count));
var_dump($count);

var_dump(str_ireplace("tt", "aa", "ttttTttttttttTT"));
var_dump(str_ireplace_with_count("tt", "aa", "ttttTttttttttTT", inout $count));
var_dump($count);

var_dump(str_ireplace("tt", "aaa", "ttttTttttttttTT"));
var_dump(str_ireplace_with_count("tt", "aaa", "ttttTttttttttTT", inout $count));
var_dump($count);

var_dump(str_ireplace("tt", "aaa", "ttttTttttttttTT"));
var_dump(str_ireplace_with_count("tt", "aaa", "ttttTttttttttTT", inout $count));
var_dump($count);

var_dump(str_ireplace(array("tt", "tt"), "aaa", "ttttTttttttttTT"));
var_dump(str_ireplace(array("tt", "tt"), array("aaa"), "ttttTttttttttTT"));
var_dump(str_ireplace(array("tt", "y"), array("aaa", "bbb"), "ttttTttttttttTT"));

var_dump(str_ireplace(array("tt", "tt"), "aaa", "ttttTttttttttTT"));
var_dump(str_ireplace(array("tt", "tt"), array("aaa"), "ttttTttttttttTT"));
var_dump(str_ireplace(array("tt", "y"), array("aaa", "bbb"), "ttttTttttttttTT"));

var_dump(str_ireplace(array("tt", "y"), array("aaa", "bbb"), array("ttttTttttttttTT", "aayyaayasdayYahsdYYY")));
var_dump(str_ireplace(array("tt", "y"), array("aaa", "bbb"), array("key"=>"ttttTttttttttTT", "test"=>"aayyaayasdayYahsdYYY")));
var_dump(str_ireplace(array("t"=>"tt", "y"=>"y"), array("a"=>"aaa", "b"=>"bbb"), array("key"=>"ttttTttttttttTT", "test"=>"aayyaayasdayYahsdYYY")));

/* separate testcase for str_ireplace() off-by-one */

$Data = "Change tracking and management software designed to watch
	for abnormal system behavior.\nSuggest features, report bugs, or ask
	questions here.";
var_dump($Data = str_ireplace("\r\n", "<br>", $Data));
var_dump($Data = str_ireplace("\n", "<br>", $Data));


echo "Done\n";
}
