<?hh <<__EntryPoint>> function main(): void {
ob_start();
echo "foo\n";
ob_end_flush();
var_dump(ob_get_level());
}
