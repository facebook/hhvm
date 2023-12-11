<?hh <<__EntryPoint>> function main(): void {
$a = dict["flags"=>(string)FILTER_FLAG_ALLOW_HEX, "options" => dict["min_range"=>"0", "max_range"=>"1024"]];
$ret = filter_var("0xff", FILTER_VALIDATE_INT, $a);
echo ($ret === 255 && $a["options"]["min_range"] === "0")?"ok\n":"bug\n";
echo ($ret === 255 && $a["options"]["max_range"] === "1024")?"ok\n":"bug\n";
echo ($ret === 255 && is_string($a["flags"]) && HH\Lib\Legacy_FIXME\eq($a["flags"], FILTER_FLAG_ALLOW_HEX))?"ok\n":"bug\n";
$a = (string)FILTER_FLAG_ALLOW_HEX;
$ret = filter_var("0xff", FILTER_VALIDATE_INT, $a);
echo ($ret === 255 && is_string($a) && HH\Lib\Legacy_FIXME\eq($a, FILTER_FLAG_ALLOW_HEX))?"ok\n":"bug\n";
$a = dict["test"=>dict["filter"=>(string)FILTER_VALIDATE_INT, "flags"=>FILTER_FLAG_ALLOW_HEX]];
$ret = filter_var_array(dict["test"=>"0xff"], $a);
echo ($ret["test"] === "0xff" && is_string($a["test"]["filter"]) && HH\Lib\Legacy_FIXME\eq($a["test"]["filter"], FILTER_VALIDATE_INT))?"ok\n":"bug\n";
echo ($ret["test"] === "0xff" && is_int($a["test"]["flags"]) && $a["test"]["flags"] == FILTER_FLAG_ALLOW_HEX)?"ok\n":"bug\n";
$a = dict["test"=>(string)FILTER_VALIDATE_INT];
$ret = filter_var_array(dict["test"=>"255"], $a);
echo ($ret["test"] === "255" && is_string($a["test"]) && HH\Lib\Legacy_FIXME\eq($a["test"], FILTER_VALIDATE_INT))?"ok\n":"bug\n";
}
