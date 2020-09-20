<?hh
<<__EntryPoint>> function main(): void {
$ini_file = dirname(__FILE__)."/parse_ini_booleans.data";

var_dump(parse_ini_file($ini_file, true));

echo "Done.\n";
}
