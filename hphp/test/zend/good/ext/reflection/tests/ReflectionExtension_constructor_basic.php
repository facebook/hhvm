<?hh <<__EntryPoint>> function main(): void {
$obj = new ReflectionExtension('reflection');
$test = $obj is ReflectionExtension;
var_dump($test);
echo "==DONE==";
}
