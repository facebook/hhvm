<?hh <<__EntryPoint>> function main(): void {
ob_start();
ReflectionExtension::export("reflection", true);
$test = ob_get_clean();
var_dump(!($test ?? false));
unset($test);
ob_start();
ReflectionExtension::export("reflection", false);
$test = ob_get_clean();
var_dump(!($test ?? false));
echo "==DONE==";
}
