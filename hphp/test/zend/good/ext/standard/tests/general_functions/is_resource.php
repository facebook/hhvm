<?hh <<__EntryPoint>> function main(): void {
$f = fopen(__FILE__, 'r');
fclose($f);
var_dump(is_resource($f));
}
