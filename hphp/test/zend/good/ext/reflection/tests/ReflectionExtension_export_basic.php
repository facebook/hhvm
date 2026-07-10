<?hh <<__EntryPoint>> function main(): void {
ob_start();
(new ReflectionExtension("reflection"))->__toString();
$test = ob_get_clean();
var_dump(!($test ?? false));
unset($test);
ob_start();
echo (new ReflectionExtension("reflection"))->__toString();
$test = ob_get_clean();
var_dump(!($test ?? false));
echo "==DONE==";
}
