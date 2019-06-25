<?hh <<__EntryPoint>> function main(): void {
$obj = new ReflectionExtension('reflection');
$test = $obj instanceof ReflectionExtension;
var_dump($test);
echo "==DONE==";
}
